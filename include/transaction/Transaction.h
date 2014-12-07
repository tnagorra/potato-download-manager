// File Transaction.h
// Contains the class Transaction

#ifndef __CO_TRANSACTION__
#define __CO_TRANSACTION__

#include <boost/asio/ssl.hpp>

#include "transaction/BasicTransaction.h"
#include "transaction/SocketTypes.h"

using boost::asio::ip::tcp;

// class Transaction (abstract)
// This class partially implements the abstract interface defined in
// BasicTransaction. It implements socket-related functionality,
// but not protocol-related functionality. Protocol-level
// implementation is provided by the subclasses fo this class.
template <typename SocketType>
class Transaction : public BasicTransaction {

    // There are only two types of tcp sockets - those which carry
    // plain data and those with SSL enabled.
    public:
    typedef typename SockTraits<SocketType>::antiSock antiSockType;

    protected:
        // A Socket for the connection
        SocketType* mptr_socket;

        // Subfunctions for the download worker. These execute
        // sequentially in the mptr_thread, and perform discrete
        // subtasks in the downloading process.

        // Perform dns resolution on the remote host. Entry point
        // for resolveHost main.
        void resolveHost();
        // Do the actual dns resolution.
        void resolveHostMain();
        // Establish a tcp connection with the remote host.
        void connectHost();

    public:
        // Constructor.
        Transaction(RemoteData* rdata, Range range = Range(0,0));

        // Destructor.
        virtual ~Transaction() {}

        // An assignment operator for converting from a transaction
        // of one socket type to another. To be used **only** for
        // redirects, nowhere else, because only the relevant members
        // are copied.
        void operator=(const Transaction<antiSockType>& i);

        // Getter for the socket
        SocketType* p_socket() const;

        // Inject a tcp socket to the downloader. These may be called
        // if you have a live socket and want that to be used for the
        // download. Reusing active sockets saves the latency time
        // losses caused when creating new connections. injectSocket
        // may only be called before you start() the download, other-
        // wise it will just return.
        void injectSocket(SSLSock* sock);
        void injectSocket(PlainSock* sock);

        // If mptr_socket is NULL, create a new one.
        void checkSocket();
};

typedef Transaction<PlainSock> PlainTransaction;
typedef Transaction<SSLSock> SSLTransaction;

#endif
// End File Transaction.h
