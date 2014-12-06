#include "transaction/BasicTransaction.h"
// The factory method needs our children to be fully defined
#include "transaction/HttpTransaction.h"

BasicTransaction::BasicTransaction(RemoteData* rdata, Range range)
    : m_range(range),
    mptr_rdata(rdata),
    mptr_thread(NULL),
    mptr_speedThread(NULL),
    m_state(State::idle),
    m_bytesTotal(range.ub()-range.lb()),
    m_bytesDone(0),
    m_beenSplit(false),
    m_beenPaused(false) {
    mptr_response = new boost::asio::streambuf;
    // If the range has a zero size but is not an unitialized range,
    // the download is immediately complete
    if (!m_range.uninitialized() && m_range.size()==0)
        m_state = State::complete;
}

BasicTransaction* BasicTransaction::factory(RemoteData* rdata,
        Range range) {
    if (!rdata)
        Throw(ex::Invalid, "The RemoteData pointer passed");
    BasicTransaction* product;
    switch (rdata->scheme()) {
        case RemoteData::Protocol::http:
            product = new HttpTransaction<PlainSock>(
                    dynamic_cast<RemoteDataHttp*>(rdata),range);
            break;
        case RemoteData::Protocol::https:
            product = new HttpTransaction<SSLSock>(
                    dynamic_cast<RemoteDataHttp*>(rdata),range);
            break;
        default:
            Throw(ex::download::BadProtocolScheme,rdata->schemeCStr());
    }
    return product;
}

BasicTransaction* BasicTransaction::factory(std::string url,
        Range range) {
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
    return m_bytesTotal;
}

uintmax_t BasicTransaction::bytesRemaining() const {
    return m_bytesTotal-m_bytesDone;
}

bool BasicTransaction::isComplete() const {
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

boost::thread* BasicTransaction::p_speedThread() const {
    return mptr_speedThread;
}

boost::function<void (std::istream&, uintmax_t)>
BasicTransaction::reader() const {
    return m_reader;
}

tcp::resolver::iterator BasicTransaction::endpIterator() const {
    return m_endpIterator;
}

bool BasicTransaction::isRunning() const {
    return (m_state!=State::idle && !isComplete());
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

BasicTransaction* BasicTransaction::clone(Range r, PlainSock* sock) {
    BasicTransaction* bt = factory(mptr_rdata,r);
    bt->injectSocket(sock);
    return bt;
}

BasicTransaction* BasicTransaction::clone(Range r, SSLSock* sock) {
    BasicTransaction* bt = factory(mptr_rdata,r);
    bt->injectSocket(sock);
    return bt;
}

void BasicTransaction::updateRange(uintmax_t u) {
    if (!m_range.uninitialized()) {
        if (u<m_range.ub())
            m_beenSplit = true;
    } else {
        if (u<m_bytesTotal)
            m_beenSplit = true;
    }
    m_range.update(u,m_range.lb());
    m_bytesTotal = m_range.size();
}

void BasicTransaction::join() const {
    if (m_state==State::idle || isComplete())
        return;
    if (mptr_thread==NULL || mptr_speedThread==NULL)
        return;
    mptr_thread->join();
    mptr_speedThread->join();
}

void BasicTransaction::speedWorker() try {
    uintmax_t tick=0;
    uintmax_t oldBytes,delta;

    while (state()!=State::downloading)
        boost::this_thread::sleep(
            boost::posix_time::milliseconds(100));

    while (!isComplete()) {
        oldBytes = bytesDone();
        boost::this_thread::sleep(
                boost::posix_time::milliseconds(500));
        delta = bytesDone()-oldBytes;
        tick++;
        m_avgSpeed = (1.0*bytesDone())/(10*tick);
        m_instSpeed = (1.0*delta)/10;
    }
} catch (std::exception& ex) {
    print(ex.what());
}

double BasicTransaction::avgSpeed() const {
    return m_avgSpeed;
}

double BasicTransaction::instSpeed() const {
    return m_instSpeed;
}

double BasicTransaction::hifiSpeed() const {
    return m_hifiSpeed;
}

double BasicTransaction::speed() const {
    return avgSpeed();
}

uintmax_t BasicTransaction::timeRemaining() const {
    if (speed()==0)
        return UINT_MAX;
    return bytesRemaining()/speed();
}

// End file BasicTransaction.cpp
