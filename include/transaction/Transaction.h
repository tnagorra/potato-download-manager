// File Transaction.h
// Contains the class Transaction

#ifndef __CO_TRANSACTION__
#define __CO_TRANSACTION__

#include <boost/asio/ssl.hpp>

#include "transaction/BasicTransaction.h"
#include "transaction/SocketTypes.h"

using boost::asio::ip::tcp;

// class Transaction
// This class is the core of the downloader, in the sense that the
// actual downloading happens and is co-ordinated here.
template <typename SocketType>
class Transaction : public BasicTransaction {

    public:
    typedef typename SockTraits<SocketType>::antiSock antiSockType;

    // Protected data members
    protected:
        // A Socket for the connection
        SocketType* mptr_socket;

        bool socketAlive();

        void createSocket();
        void resolveHost();
        void resolveHostMain();
        void connectHost();

        /*using BasicTransaction::m_state;
        using BasicTransaction::m_range;
        using BasicTransaction::mptr_rdata;
        using BasicTransaction::m_thread;
        using BasicTransaction::m_response;
        using BasicTransaction::m_reader;
        using BasicTransaction::m_endpIterator;
        using BasicTransaction::m_bytesTotal;
        using BasicTransaction::m_bytesDone;*/

    public:
        // Constructor. Default Range argument (0,0) means the
        // entire resource
        Transaction(RemoteData* rdata, SocketType* sock = NULL,
                const Range& range = Range(0,0));

        // Overload in case a different socket type is provided
        Transaction(RemoteData* rdata, antiSockType* asock,
                const Range& range = Range(0,0));

        // Destructor
        virtual ~Transaction() {}

        // The cross-assignment operator.
        void operator=(const Transaction<antiSockType>& i);

        SocketType* p_socket() const;
};

typedef Transaction<PlainSock> PlainTransaction;
typedef Transaction<SSLSock> SSLTransaction;

#endif
// End File Transaction.h
