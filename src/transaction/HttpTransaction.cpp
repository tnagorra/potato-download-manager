#include <iostream>
#include <algorithm>
#include <functional>

#include <boost/lexical_cast.hpp>
#include <boost/chrono.hpp>
#include "common/helper.h"
#include "transaction/HttpTransaction.h"

using boost::asio::ip::tcp;

class Redirect {
public:
    HttpTransaction<SSLSock>* m_rstp_secure;
    HttpTransaction<PlainSock>* m_rstp_plain;
    Redirect(HttpTransaction<SSLSock>* restartp) {
        m_rstp_secure = restartp;
        m_rstp_plain = NULL;
    }
    Redirect(HttpTransaction<PlainSock>* restartp) {
        m_rstp_plain = restartp;
        m_rstp_secure = NULL;
    }
};

// Constructor.
template <typename SocketType>
HttpTransaction<SocketType>::HttpTransaction(RemoteDataHttp* rdata,
        const Range& range, unsigned attempts, unsigned wait)
    : Transaction<SocketType>(rdata,range, attempts, wait) {
}

// Start the downloader thread and return immediately
template <typename SocketType>
void HttpTransaction<SocketType>::start() {
    if (m_state!= State::idle)
        return;
    if (mptr_socket==NULL)
        mptr_socket = SockTraits<SocketType>::transform();
    this->mptr_thread = new boost::thread(
        &HttpTransaction<SocketType>::workerMain, this);
    this->mptr_speedThread = new boost::thread(
        &BasicTransaction::speedWorker, this);
}

// Stop the download if it is running.
template <typename SocketType>
void HttpTransaction<SocketType>::stop() {
    if(this->mptr_thread && this->mptr_thread->joinable())
        this->mptr_thread->interrupt();
    if(this->mptr_speedThread && this->mptr_speedThread->joinable())
        this->mptr_speedThread->interrupt();
}

// workerMain() is the entry point of the download Thread
template <typename SocketType>
void HttpTransaction<SocketType>::workerMain() try {
    resolveHost();
    connectHost();
    createAndSendRequest();
    receiveHeaders();
    writeOut();
} catch (Redirect& redir) {
    m_state = State::requesting;
    if (redir.m_rstp_secure==NULL) {
        redir.m_rstp_plain->workerMain();
    } else {
        redir.m_rstp_secure->workerMain();
    }

} catch (std::exception& exc) {
    m_state = State::failed;
    mptr_exbridge->log(exc);
    // TODO: don't show here but the exception bridge isn't working
    show(exc.what());
}

// Create the http request headers and send them
template <typename SocketType>
void HttpTransaction<SocketType>::createAndSendRequest() {
    readToSink();
    // A streambuf for buffering the request
    boost::asio::streambuf request;
    // And a stream to write to it
    std::ostream rqstream(&request);
    // Set state
    m_state = State::requesting;

    // Write the starting line
    rqstream<<dynamic_cast<RemoteDataHttp*>(mptr_rdata)->method()<<
            " "<<mptr_rdata->path()<<" HTTP/1.1\r\n";
    // And the host-line
    rqstream<<"Host: "<<mptr_rdata->server()<<"\r\n";

    // Other defaults can be overridden by headers provided
    // in RemoteDataHttp.
    // TODO make this changeable (don't hard-code)
    if(dynamic_cast<RemoteDataHttp*>(mptr_rdata)->header("Accept")=="")
        rqstream<<"Accept: */*\r\n";
    //if (dynamic_cast<RemoteDataHttp*>(mptr_rdata)->header("Connection")=="")
    //rqstream<<"Connection: \r\n";
    if (!m_range.uninitialized())
        rqstream<<"Range: bytes="<<m_range.lb()<<"-"<<m_range.ub()-1
                <<"\r\n";

    // Now spew the given headers
    std::map<std::string,std::string> headers_map;
    headers_map = dynamic_cast<RemoteDataHttp*>(mptr_rdata)->headers();

    for (std::map<std::string,std::string>::iterator
            it=headers_map.begin(); it!=headers_map.end(); it++)
        rqstream<<it->first<<": "<<it->second<<"\r\n";
    rqstream<<"\r\n";   // The final empty line
    // Can be interrupted here
    boost::this_thread::interruption_point();
    // Make the request!
    try {
        boost::asio::write(*mptr_socket, request);
    } catch (boost::system::system_error& err) {
        Throw(ex::download::ErrorSendingRequest,err.what());
    }
    m_state = State::waiting;
    // And here too
    boost::this_thread::interruption_point();
}

// Wait while the expecting data from the server
template <typename SocketType>
void HttpTransaction<SocketType>::waitData() {
    // While there are no bytes to read, just hang around
    while (!mptr_socket->available()) {
        // a sleep_for call is also an interruption point.
        boost::this_thread::sleep(
            boost::posix_time::milliseconds(100));
    }
}

template<>  // Template specialization for SSLSock
void HttpTransaction<SSLSock>::waitData() {
    // While there are no bytes to read, just hang around
    while (!mptr_socket->lowest_layer().available()) {
        // a sleep_for call is also an interruption point.
        boost::this_thread::sleep(
            boost::posix_time::milliseconds(100));
    }
}

// Read and parse the http response headers
template <typename SocketType>
void HttpTransaction<SocketType>::receiveHeaders() {
    // Wait for the headers to arrive
    waitData();

    m_state = State::ready;
    // Once bytes are available, read some (the status line)
    boost::asio::read_until(*mptr_socket, *mptr_response, "\r\n");
    boost::this_thread::interruption_point();

    // Extract parts from the status line
    std::istream resp_strm(mptr_response);

    std::string http_version;
    resp_strm>>http_version;
    unsigned int status_code;
    resp_strm>>status_code;
    std::string status_message;
    std::getline(resp_strm, status_message);

    m_statusLine = http_version+" "+
                   boost::lexical_cast<std::string>(status_code)+status_message;

    if (!resp_strm || http_version.substr(0,5)!="HTTP/")
        Throw(ex::download::BadResponse);

    // Read of all the headers to the buffer
    boost::asio::read_until(*mptr_socket, *mptr_response, "\r\n\r\n");
    boost::this_thread::interruption_point();

    std::string header;
    while(std::getline(resp_strm, header) && header!="\r") {
        boost::to_lower(header);
        size_t colon = header.find(':');
        if(colon == std::string::npos)
            continue;
        m_respHeaders[header.substr(0,colon)]
            = header.substr(colon+2,header.size()-colon-3);
    }

    handleStatusCode(status_code);

    // If the transfer encoding is chunked, bail out.
    if (m_respHeaders.count("transfer-encoding")>0) {
        if (m_respHeaders["transfer-encoding"]=="chunked")
            Throw(ex::download::Chunked);
    }

    uintmax_t bytesTotal = 0;
    // Determine the size of the response body
    if (m_respHeaders.count("content-length")>0) {
        bytesTotal = boost::lexical_cast<uintmax_t>(
                         m_respHeaders["content-length"]);
    }

    // Find filename from headers
    if(m_respHeaders.count("content-disposition")>0) {
        std::string disposition = boost::lexical_cast<std::string>(
                                      m_respHeaders["content-disposition"]);

        File f("newfile");
        f.append(disposition);

        size_t equalto = disposition.find_first_of('=');
        size_t length = disposition.length();
        size_t first = disposition.find_first_of('"',equalto);
        size_t last = disposition.find_first_of('"',first+1);


        if( equalto!=std::string::npos){

        // Format: Content-Disposition: attachment; filename="fname.ext"
        // If apostrophe is present
        if(first!=std::string::npos && last>0)
            disposition = disposition.substr(first +1,last-first+1 -2);
        else
            disposition = disposition.substr(equalto +1, length-equalto+1 -1);

        // Fill filename in remotedata
        // TODO: save this somewhere permanently
        mptr_rdata->filename(disposition);
        }
    }

    // Find out whether byterange requests are supported
    if (m_respHeaders.count("accept-ranges")>0) {
        mptr_rdata->canPartial(
            (m_respHeaders["accept-ranges"]=="bytes")?
            RemoteData::Partial::yes : RemoteData::Partial::no);
    } else if (!m_range.uninitialized()) {
        if (m_respHeaders.count("content-length")>0)
            mptr_rdata->canPartial(
                (bytesTotal==m_range.size())?
                RemoteData::Partial::yes : RemoteData::Partial::no);
        else if (status_code==206)
            mptr_rdata->canPartial(RemoteData::Partial::yes);
    }
    // NOTE
    // bytesTotal is zero if content length isn't given
    // and m_range is kept uninitialized
    if(m_range.uninitialized() && bytesTotal!=0)
        m_range.update(bytesTotal,0);

}

// Handle an unexpected http status
template <typename SocketType>
void HttpTransaction<SocketType>::handleStatusCode(unsigned int code) {
    unsigned int category = code/100;
    // 1xx is informational, 4xx is client error, 5xx server error
    // 1xx could be processed further but now we dont
    if (category==1 || category==4 || category==5) {
        Throw(ex::download::BadStatusCode,code);
    } else if (category==3) {
        // Redirect
        std::string location = m_respHeaders["location"];
        RemoteData* rd = RemoteData::factory(location);
        readToSink();

        clearProgress();
        if (rd->scheme()==mptr_rdata->scheme()) {
            // We need the old servername, to find out if a new
            // connection needs to be made.
            std::string oldServer = mptr_rdata->server();
            // Set fullUrl to the new location
            mptr_rdata->fullUrl(location);
            // If we have been redirected to a different server,
            // everything needs to be restarted
            if (oldServer!=mptr_rdata->server()) {
                mptr_rdata->canPartial(RemoteData::Partial::unknown);
                delete mptr_socket;
                mptr_socket = SockTraits<SocketType>::transform();
            }
            // Throwout, to restart our workerMain
            throw Redirect(this);
        } else {
            RemoteDataHttp* rdx = dynamic_cast<RemoteDataHttp*>(rd);

            HttpTransaction<SocketType> old = *this;
            HttpTransaction<antiSockType>* antiSock =
                new(this) HttpTransaction<antiSockType>(rdx, m_range);

            *dynamic_cast<Transaction<antiSockType>*>(antiSock) =
                *dynamic_cast<Transaction<SocketType>*>(&old);

            throw Redirect(antiSock);
        }
    }
}

// write data to somewhere through the callback - the *real*
// downloader!
template <typename SocketType>
void HttpTransaction<SocketType>::writeOut() {

    uintmax_t bufBytes = mptr_response->size();
    std::istream writeStream(mptr_response);
    // TODO a check must be kept here!
    if (bufBytes)
        m_reader(writeStream,0,bufBytes);
    m_bytesDone = bufBytes;
    m_state = State::downloading;

    //show(m_bytesDone);
    boost::system::error_code error;
    while ((bufBytes = boost::asio::read(*mptr_socket, *mptr_response, boost::asio::transfer_at_least(1), error))) {

        // This blocks prevents writing to a file
        // when splitting is performed
        while (m_pauseRequest)
            boost::this_thread::sleep(
                boost::posix_time::milliseconds(200));
        m_pauseRequest = false;

        // when range is not resumable
        if (!m_range.uninitialized() && bufBytes+m_bytesDone >= m_range.size()) {
            if (m_bytesDone>m_range.size())
                Throw(ex::Error, "not recoverable situation");
            m_reader(writeStream,0,m_range.size()-m_bytesDone);
            m_bytesDone = m_range.size();
            error=boost::asio::error::eof;
            break;
        } else {
            m_reader(writeStream,0,bufBytes);
        }
        m_bytesDone += bufBytes;

        boost::this_thread::interruption_point();
    }
    //show(m_bytesDone);

    if (error!=boost::asio::error::eof) {
        m_state = State::failed;
    }

    m_state = State::complete;
    m_pauseRequest = false;
}

// Read out any remaining data on the socket to sink
template <typename SocketType>
void HttpTransaction<SocketType>::readToSink() {
    boost::asio::streambuf dumie;
    if (mptr_socket->available())
        boost::asio::read(*mptr_socket, dumie,
                          boost::asio::transfer_at_least(
                              mptr_socket->available()));
}

template <> // Template specialization for SSLSock
void HttpTransaction<SSLSock>::readToSink() {
    boost::asio::streambuf dumie;
    if (mptr_socket->lowest_layer().available())
        boost::asio::read(*mptr_socket, dumie,
                          boost::asio::transfer_at_least(
                              mptr_socket->lowest_layer().available()));
}

// Clear all transaction progress and return to start
// state. Needed to handle things like redirects.
template <typename SocketType>
void HttpTransaction<SocketType>::clearProgress() {
    m_state = State::idle;
    m_bytesDone = 0;
    m_statusLine = "";
    //m_beenSplit = false;
    m_respHeaders = std::map<std::string,std::string>();
    delete mptr_response;
    mptr_response = new boost::asio::streambuf;
}

template class HttpTransaction<PlainSock>;
template class HttpTransaction<SSLSock>;

// End file HttpTransaction.cpp


