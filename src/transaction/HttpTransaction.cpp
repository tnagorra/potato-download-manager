#include <iostream>
#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>

#include <boost/lexical_cast.hpp>
#include <boost/chrono.hpp>

#include "transaction/HttpTransaction.h"

using boost::asio::ip::tcp;
template class HttpTransaction<PlainSock>;
template class HttpTransaction<SSLSock>;

template <typename SocketType>
HttpTransaction<SocketType>::HttpTransaction(RemoteDataHttp* rdata,
        SocketType* sock, const Range& range)
    : Transaction<SocketType>(rdata,sock,range)
{ }

template <typename SocketType>
HttpTransaction<SocketType>::HttpTransaction(RemoteDataHttp* rdata,
        antiSockType* sock, const Range& range)
    : Transaction<SocketType>(rdata,sock,range)
{ }

template <typename SocketType>
void HttpTransaction<SocketType>::start() {
    this->mptr_thread = new boost::thread(
            &HttpTransaction<SocketType>::workerMain, this);
}

template <typename SocketType>
void HttpTransaction<SocketType>::stop() {
    this->mptr_thread->interrupt();
}

template <typename SocketType>
void HttpTransaction<SocketType>::fuckyou() {
    boost::asio::streambuf dumie;
    if (mptr_socket->available())
        boost::asio::read(*mptr_socket, dumie,
                boost::asio::transfer_at_least(
                    mptr_socket->available()));
}

template <>
void HttpTransaction<SSLSock>::fuckyou() {
    boost::asio::streambuf dumie;
    if (mptr_socket->lowest_layer().available())
        boost::asio::read(*mptr_socket, dumie,
                boost::asio::transfer_at_least(
                    mptr_socket->lowest_layer().available()));
}

template <typename SocketType>
void HttpTransaction<SocketType>::createAndSendRequest() {
    fuckyou();
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
    if (dynamic_cast<RemoteDataHttp*>(mptr_rdata)->header("Connection")=="")
        rqstream<<"Connection: close\r\n";

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

template <typename SocketType>
void HttpTransaction<SocketType>::waitData() {
    // While there are no bytes to read, just hang around
    while (!mptr_socket->available()) {
        // a sleep_for call is also an interruption point.
        boost::this_thread::sleep_for(
                boost::chrono::milliseconds(100));
    }
}

template<>
void HttpTransaction<SSLSock>::waitData() {
    // While there are no bytes to read, just hang around
    while (!mptr_socket->lowest_layer().available()) {
        // a sleep_for call is also an interruption point.
        boost::this_thread::sleep_for(
                boost::chrono::milliseconds(100));
    }
}

template <typename SocketType>
void HttpTransaction<SocketType>::receiveHeaders() {
    // Wait for the headers to arrive
    waitData();

    if (SockTraits<SocketType>().ssl)
        std::cout<<"\n\nWe have ssl\n\n";
    else
        std::cout<<"\n\nWe D0N'T have ssl\n\n";


    m_state = State::ready;
    // Once bytes are available, read some (the status line)
    boost::asio::read_until(*mptr_socket, *mptr_response, "\r\n");
    boost::this_thread::interruption_point();

    // Extract parts from the status line
    std::istream resp_strm(mptr_response);
    std::string http_version;
    if (false ) {
        std::getline(resp_strm, http_version);
        std::cout<<"hereyougo\n"<<http_version<<std::endl;
        Throw(ex::download::BadResponse);
    }
    resp_strm>>http_version;
    unsigned int status_code;
    resp_strm>>status_code;
    std::string status_message;
    std::getline(resp_strm, status_message);

    if (!resp_strm || http_version.substr(0,5)!="HTTP/") {
        Throw(ex::download::BadResponse);
    }

    // Do this handling more elegantly
    //if (status_code!=200)
    //    Throw(ex::download::BadStatusCode,status_code);

    m_statusLine = http_version+" "+
        boost::lexical_cast<std::string>(status_code)+status_message;
    print(m_statusLine);

    // Read of all the headers to the buffer
    boost::asio::read_until(*mptr_socket, *mptr_response, "\r\n\r\n");
    boost::this_thread::interruption_point();

    std::string header; size_t colon;
    while(std::getline(resp_strm, header) && header!="\r") {
        print(header);
        if ((colon = header.find(':'))!=std::string::npos) {
            m_respHeaders[boost::to_lower_copy(header.substr(0,colon))]
                = header.substr(colon+2,header.size()-colon-3);
        }
    }

    handleStatusCode(status_code);
}

template <typename SocketType>
void HttpTransaction<SocketType>::handleStatusCode(unsigned int code) {
    unsigned int category = code/100;
    // 1xx is informational, 4xx is client error, 5xx server error
    // 1xx could be processed further but now we dont
    if (category==1 || category==4 || category==5) {
        Throw(ex::download::BadStatusCode,code);
    }
    else if (category==3) {
        // Redirect
        std::string location = m_respHeaders["location"];
        RemoteData* rd = RemoteData::factory(location);
        fuckyou();

        clearProgress();
        if (rd->scheme()==mptr_rdata->scheme()) {
            mptr_rdata->fullUrl(location);

        } else {
            RemoteDataHttp* rdx = dynamic_cast<RemoteDataHttp*>(rd);

            HttpTransaction<SocketType> old = *this;
            HttpTransaction<antiSockType>* antiSock =
                new(this) HttpTransaction<antiSockType>(
                        rdx, mptr_socket, m_range);

            *dynamic_cast<Transaction<antiSockType>*>(antiSock) =
                *dynamic_cast<Transaction<SocketType>*>(&old);
            throw antiSock;
            //Transaction<antiSockType>* btths =
            //    reinterpret_cast<Transaction<antiSockType>*>(this);
            //Transaction<SocketType>* btold =
            //    reinterpret_cast<Transaction<SocketType>*>(&old);
            //BasicTransaction* btths = this;
            //BasicTransaction* btoth = antiSock;
            //*btths = *btold;
            //*this=old;
        // dafuq?
        // TODO
        // Things are risky here
        }
        m_state = State::requesting;
    }
}

template <typename SocketType>
void HttpTransaction<SocketType>::clearProgress() {
    m_statusLine = "";
    m_respHeaders = std::map<std::string,std::string>();
    mptr_response = new boost::asio::streambuf;
}

template <typename SocketType>
void HttpTransaction<SocketType>::writeOut() {
    size_t bufBytes = mptr_response->size();
    if (bufBytes)
        m_reader(*mptr_response);
    m_bytesDone = bufBytes;
    m_state = State::downloading;
    boost::system::error_code error;
    while (bufBytes = boost::asio::read(*mptr_socket, *mptr_response,
                boost::asio::transfer_at_least(1), error)) {
        m_reader(*mptr_response);
        m_bytesDone += bufBytes;
        boost::this_thread::interruption_point();
    }
    if (error!=boost::asio::error::eof) {
        m_state = State::failed;
    }
    m_state = State::complete;
}

template <typename SocketType>
void HttpTransaction<SocketType>::workerMain() try {
    resolveHost();
    connectHost();
    createAndSendRequest();
    receiveHeaders();
    writeOut();
} catch (HttpTransaction<antiSockType>* radar) {
    radar->workerMain();
}
//} catch (boost::thread_interrupted& dummy) { std::cout<<";)\n"; }

// End file HttpTransaction.cpp
