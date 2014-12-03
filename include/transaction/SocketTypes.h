// File SocketTypes.h
// Contains

#ifndef __CO_SOCKETTYPES__
#define __CO_SOCKETTYPES__

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

using boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;

// The two types of sockets,
// A SSL-secured socket
typedef ssl::stream<tcp::socket> SSLSock;
// And a plain socket.
typedef tcp::socket PlainSock;

// A traits struct for the Sock classes.
template <typename SocketType> struct SockTraits;

// TODO TODO
// separate the socket-creation and management code from
// the traits structure.

template <>
struct SockTraits<SSLSock> {
    typedef PlainSock antiSock;
    typedef SSLSock thisSock;
    bool ssl;

    SockTraits() : ssl(true) {}

    static SSLSock* transform(PlainSock* psock=NULL) {
        boost::asio::io_service* ios;
        if (false && psock!=NULL)
            ios = &psock->get_io_service();
        else
            ios = new boost::asio::io_service;

        ssl::context ctx(ssl::context::sslv23);
        ctx.set_default_verify_paths();

        SSLSock* ssock = new SSLSock(*ios,ctx);
        return ssock;
    }

    static SSLSock* transform(SSLSock* ssock) {
        return ssock;
    }
};

// The specialization for nonSSL sockets.
template <>
struct SockTraits<PlainSock> {
    typedef SSLSock antiSock;
    typedef PlainSock thisSock;

    bool ssl;

    SockTraits() : ssl(false) {}

    static PlainSock* transform(SSLSock* ssock=NULL) {
        boost::asio::io_service* ios;
        if (ssock!=NULL)
            ios = &ssock->get_io_service();
        else
            ios = new boost::asio::io_service;

        PlainSock* psock = new PlainSock(*ios);
        return psock;
    }

    static PlainSock* transform(PlainSock* psock) {
        return psock;
    }

};

#endif
// End File SocketTypes.h
