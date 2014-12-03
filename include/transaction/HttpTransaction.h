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
// The contract this class fulfills is to take a RemoteDataHttp and
// a Range, and produce a stream of bytes.
// This class is the core of the downloader, in the sense that the
// actual downloading happens and is co-ordinated here.
template <typename SocketType>
class HttpTransaction : public Transaction<SocketType> {

    // Private data members
    private:
        // The response status line
        std::string m_statusLine;
        // The response headers
        std::map<std::string,std::string> m_respHeaders;

        // these are smaller functions that do specific actions
        // inside the worker thread
        void createAndSendRequest();
        void readToSink();
        void waitData();
        void receiveHeaders();
        void handleStatusCode(unsigned int code);
        void writeOut();

        // Clear all transaction progress and return to start
        // state
        void clearProgress();

        // we will USE these base class members
        using typename Transaction<SocketType>::State;
        using Transaction<SocketType>::m_state;
        using Transaction<SocketType>::m_range;
        using Transaction<SocketType>::mptr_thread;
        using Transaction<SocketType>::mptr_socket;
        using Transaction<SocketType>::mptr_response;
        using Transaction<SocketType>::mptr_rdata;
        using Transaction<SocketType>::m_bytesDone;
        using Transaction<SocketType>::m_bytesTotal;
        using Transaction<SocketType>::m_reader;
        using Transaction<SocketType>::resolveHost;
        using Transaction<SocketType>::connectHost;
        using Transaction<SocketType>::m_beenSplit;
        using Transaction<SocketType>::m_beenPaused;

    public:
        typedef typename SockTraits<SocketType>::antiSock antiSockType;

        // workerMain() is the entry point of the worker thread
        void workerMain();

        // Constructor. Default Range argument (0,0) means the
        // entire resource
        HttpTransaction(RemoteDataHttp* rdata, SocketType* sock = NULL,
                const Range range = Range(0,0));

        // This has been deemed unnecessary for now
        // Overload in case a different socket type is provided
        //HttpTransaction(RemoteDataHttp* rdata, antiSockType* asock,
        //        const Range range = Range(0,0));

        void start();
        void stop();
};

typedef HttpTransaction<PlainSock> HttppTransaction;
typedef HttpTransaction<SSLSock> HttpsTransaction;

#endif
// End File HttpTransaction.h
