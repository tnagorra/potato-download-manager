#include "transaction/BasicTransaction.h"
// The factory method needs our children to be fully defined
#include "transaction/HttpTransaction.h"

BasicTransaction::BasicTransaction(RemoteData* rdata,
        const Range& range)
    : m_range(range),
    mptr_rdata(rdata),
    mptr_thread(NULL),
    m_state(State::idle),
    m_bytesTotal(range.ub()-range.lb()),
    m_bytesDone(0) { mptr_response = new boost::asio::streambuf; }

BasicTransaction* BasicTransaction::factory(RemoteData* rdata,
        const Range& range) {
    if (!rdata)
        return NULL;
    BasicTransaction* product;
    switch (rdata->scheme()) {
        case RemoteData::Protocol::http:
            product = new HttpTransaction<PlainSock>(
                    dynamic_cast<RemoteDataHttp*>(rdata),NULL,range);
            break;
        case RemoteData::Protocol::https:
            product = new HttpTransaction<SSLSock>(
                    dynamic_cast<RemoteDataHttp*>(rdata),NULL,range);
            break;
        default:
            product = NULL;
    }
    return product;
}

BasicTransaction* BasicTransaction::factory(std::string url,
        const Range& range) {
    RemoteData* rdata_url = RemoteData::factory(url);
    return factory(rdata_url, range);
}

void BasicTransaction::registerReader(
        boost::function<void (boost::asio::streambuf&)>& bytereader) {
    m_reader = bytereader;
}

std::string BasicTransaction::stateString () const{
    switch(m_state) {
        case State::idle:
            return "Not Started";
        case State::resolving:
            return "Resolving";
        case State::connecting:
            return "Connecting";
        case State::requesting:
            return "Requesting";
        case State::waiting:
            return "Waiting";
        case State::downloading:
            return "Downloading";
        case State::complete:
            return "Complete";
        case State::ready:
            return "Ready";
        default:
            return "Enlightened";
    }
}

typename BasicTransaction::State
    BasicTransaction::state() const {
    return m_state;
}

uintmax_t BasicTransaction::bytesDone() const {
    return m_bytesDone;
}

uintmax_t BasicTransaction::bytesTotal() const {
    return m_range.size();
}

bool BasicTransaction::complete() const {
    return state()==State::complete || state()==State::failed;
}

Range BasicTransaction::range() const {
    return m_range;
}

RemoteData* BasicTransaction::p_remoteData() const {
    return mptr_rdata;
}

boost::thread* BasicTransaction::p_thread() const {
    return mptr_thread;
}

boost::function<void (boost::asio::streambuf&)>
BasicTransaction::reader() const {
    return m_reader;
}

tcp::resolver::iterator BasicTransaction::endpIterator() const {
    return m_endpIterator;
}

// End file BasicTransaction.cpp
