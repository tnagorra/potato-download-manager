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
    m_bytesDone(0),
    m_beenSplit(false) {
    mptr_response = new boost::asio::streambuf;
    if (!m_range.uninitialized() && m_range.size()==0)
        m_state = State::complete;
}

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
            Throw(ex::download::BadProtocolScheme);
    }
    return product;
}

BasicTransaction* BasicTransaction::factory(std::string url,
        const Range& range) {
    RemoteData* rdata_url = RemoteData::factory(url);
    return factory(rdata_url, range);
}

void BasicTransaction::registerReader(
        boost::function<void (std::istream&, uintmax_t)>& bytereader) {
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

boost::function<void (std::istream&, uintmax_t)>
BasicTransaction::reader() const {
    return m_reader;
}

tcp::resolver::iterator BasicTransaction::endpIterator() const {
    return m_endpIterator;
}

bool BasicTransaction::isRunning() const {
    return (m_state!=State::idle && !complete());
}

void BasicTransaction::pause() {
    m_beenPaused = true;
}

bool BasicTransaction::isPaused() const {
    return m_beenPaused;
}

void BasicTransaction::play() {
    m_beenPaused = false;
}

void BasicTransaction::updateRange(uintmax_t u) {
    if (!m_range.uninitialized()) {
        if (u>m_range.ub())
            m_beenSplit = true;
    } else {
        if (u>m_bytesTotal)
            m_beenSplit = true;
    }
    m_range.update(u,m_range.lb());
    m_bytesTotal = m_range.size();
}

// End file BasicTransaction.cpp
