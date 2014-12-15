// File HttpTransaction.h
// Contains the class HttpTransaction

#ifndef __CO_HTTPTRANSACTION__
#define __CO_HTTPTRANSACTION__

#include <iostream>
#include <string>
#include <ctime>
#include <map>

#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/function.hpp>

#include "common/ex.h"
#include "transaction/ex.h"
#include "transaction/Transaction.h"
#include "transaction/Range.h"
#include "transaction/RemoteDataHttp.h"


// class HttpTransaction
// This class is a protocol-specific subclass of Transaction. As the
// name implies, it handles the http protocol.
template <typename SocketType>
class HttpTransaction : public Transaction<SocketType> {

    // we will USE these base class members
    using typename Transaction<SocketType>::State;
    using Transaction<SocketType>::m_state;
    using Transaction<SocketType>::m_range;
    using Transaction<SocketType>::mptr_thread;
    using Transaction<SocketType>::mptr_speedThread;
    using Transaction<SocketType>::mptr_socket;
    using Transaction<SocketType>::mptr_response;
    using Transaction<SocketType>::mptr_rdata;
    using Transaction<SocketType>::m_bytesDone;
    using Transaction<SocketType>::m_bytesTotal;
    using Transaction<SocketType>::m_reader;
    using Transaction<SocketType>::m_beenSplit;
    using Transaction<SocketType>::m_pauseRequest;
    using Transaction<SocketType>::connectHost;
    using Transaction<SocketType>::resolveHost;
    using Transaction<SocketType>::resolveHostMain;
    using Transaction<SocketType>::isRunning;

    // Private data members
    private:
    // The response status line
    std::string m_statusLine;
    // The response headers, as name-value pairs
    std::map<std::string,std::string> m_respHeaders;

    // Subfunctions for the download worker. These execute
    // sequentially in the mptr_thread, and perform discrete
    // subtasks in the downloading process.

    // Create the http request headers and send them
    void createAndSendRequest();
    // Wait while the expecting data from the server
    void waitData();
    // Read and parse the http response headers
    void receiveHeaders();
    // Handle an unexpected http status code
    void handleStatusCode(unsigned int code);
    // write data to somewhere through the callback - the *real*
    // downloader!
    void writeOut();

    // Read out any remaining data on the socket to sink
    void readToSink();
    // Clear all transaction progress and return to start
    // state. Needed to handle things like redirects.
    void clearProgress();

    public:
    typedef typename SockTraits<SocketType>::antiSock antiSockType;

   // Constructor.
    HttpTransaction(RemoteDataHttp* rdata, const Range& range = Range(0,0));

    // Start the downloader thread and return immediately.
    void start();

    // Stop the download if it is running.
    void stop();

    // workerMain() is the entry point of the download Thread
    void workerMain();
};

typedef HttpTransaction<PlainSock> HttppTransaction;
typedef HttpTransaction<SSLSock> HttpsTransaction;

#endif
// End File HttpTransaction.h
