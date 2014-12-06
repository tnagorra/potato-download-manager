#include "transaction/Transaction.h"
#include <sys/time.h>
#include <sys/socket.h>

template class Transaction<PlainSock>;
template class Transaction<SSLSock>;

template <typename SocketType>
Transaction<SocketType>::Transaction(RemoteData* rdata, Range range)
    : BasicTransaction(rdata,range), mptr_socket(NULL) { }

// Dont think we really need this. And things are simple for the
// factory without it.
/*template <typename SocketType>
Transaction<SocketType>::Transaction(RemoteData* rdata,
        antiSockType* sock, const Range range)
    : BasicTransaction(rdata,range) {
    mptr_socket = SockTraits<SocketType>::transform(sock);
}*/

template <typename SocketType>
void Transaction<SocketType>::resolveHost() {
    checkSocket();
    boost::system::error_code ec;
    // Return, if we already have an endpoint (meaning we are already
    // connected).
    if (mptr_socket->remote_endpoint(ec)!=tcp::endpoint())
        return;
    else
        resolveHostMain();
}

template <>
void Transaction<SSLSock>::resolveHost() {
    checkSocket();
    boost::system::error_code ec;
    // Return, if we already have an endpoint (meaning we are already
    // connected).
    if (mptr_socket->lowest_layer().remote_endpoint(ec)!=
            tcp::endpoint()) {
        return;
    } else resolveHostMain();
}

template <typename SocketType>
void Transaction<SocketType>::resolveHostMain() {
    // Number of tries left
    uint8_t triesleft = 2;
    // In the 'resolving' state
    m_state = State::resolving;

    tcp::resolver resolver(mptr_socket->get_io_service());
    tcp::resolver::query query(mptr_rdata->server(),
            mptr_rdata->schemeCStr(), tcp::resolver::query::passive);
    tcp::resolver::iterator err_itr, temp_itr;

    // Lets try to resolve our hostname
    do {
        try {
        m_endpIterator = resolver.resolve(query);
        boost::this_thread::interruption_point();
        } catch (boost::system::system_error& err) {
            if (triesleft==1)
                Throw(ex::download::HostNotFound,mptr_rdata->server());
        }
    } while (m_endpIterator==err_itr && --triesleft);
}

template <typename SocketType>
void Transaction<SocketType>::connectHost() {
    boost::system::error_code ec;
    // Return, if we already have an endpoint (meaning we are already
    // connected).
    if (mptr_socket->remote_endpoint(ec)!=tcp::endpoint()) {
        return;
    }

    // Number of tries left
    uint8_t triesleft = 2;
    // In the 'connecting' state
    m_state = State::connecting;

    tcp::resolver::iterator err_itr, temp_itr;
    do {
        boost::this_thread::interruption_point();
        try {
            temp_itr = boost::asio::connect(*mptr_socket,
                    m_endpIterator);
        } catch (boost::system::system_error& err) { }
    } while (temp_itr==err_itr && --triesleft);

    mptr_socket->set_option(tcp::no_delay(true));
    if (temp_itr==err_itr)
        Throw(ex::download::CouldNotConnect,
                "TCP connection could not be established");

    boost::this_thread::interruption_point();
}

template <>
void Transaction<SSLSock>::connectHost() {
    boost::system::error_code ec;
    // Return, if we already have an endpoint (meaning we are already
    // connected).
    if (mptr_socket->lowest_layer().remote_endpoint(ec)!=
            tcp::endpoint()) {
        return;
    }

    // Number of tries left
    uint8_t triesleft = 2;
    // In the 'connecting' state
    m_state = State::connecting;

    tcp::resolver::iterator err_itr, temp_itr;
    do {
        boost::this_thread::interruption_point();
        try {
            temp_itr = boost::asio::connect(mptr_socket->lowest_layer()
                    , m_endpIterator);
        } catch (boost::system::system_error& err) { }
    } while (temp_itr==err_itr && --triesleft);

    if (temp_itr==err_itr)
        Throw(ex::download::CouldNotConnect,
                "TCP connection could not be established");

    mptr_socket->lowest_layer().set_option(tcp::no_delay(true));

    // Perform SSL handshake and verify the remote host's certificate
    mptr_socket->set_verify_mode(ssl::verify_peer);
    mptr_socket->set_verify_callback(
            ssl::rfc2818_verification(mptr_rdata->server()));
    mptr_socket->handshake(SSLSock::client);

    boost::this_thread::interruption_point();
}

template <typename SocketType>
SocketType* Transaction<SocketType>::p_socket() const {
    return mptr_socket;
}

template <typename SocketType>
void Transaction<SocketType>::injectSocket(SSLSock* sock) {
    mptr_socket = SockTraits<SocketType>::transform(sock);
}

template <typename SocketType>
void Transaction<SocketType>::injectSocket(PlainSock* sock) {
    mptr_socket = SockTraits<SocketType>::transform(sock);
}

template <typename SocketType>
void Transaction<SocketType>::checkSocket() {
    if (mptr_socket==NULL)
        mptr_socket = SockTraits<SocketType>::transform();
}

// An assignment operator for converting from a transaction of one
// socket type to another. To be used **only** for redirects, nowhere
// else, because only the relevant members are copied.
template <typename SocketType>
void Transaction<SocketType>::operator=(
        const Transaction<antiSockType>& i) {
    //m_state = i.state();
    m_range = i.range();
    //mptr_rdata = i.p_remoteData();
    mptr_thread = i.p_thread();
    mptr_speedThread = i.p_speedThread();
    //m_response = i.m_response;
    m_reader = i.reader();
    //m_endpIterator = i.endpIterator();
    //m_bytesTotal = i.bytesTotal();
    //m_bytesDone = i.bytesDone();
    mptr_socket = SockTraits<SocketType>::transform(i.p_socket());
}

// End file Transaction.cpp
