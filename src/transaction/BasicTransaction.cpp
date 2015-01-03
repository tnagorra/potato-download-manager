#include "transaction/BasicTransaction.h"
// The factory method needs our children to be fully defined
#include "transaction/HttpTransaction.h"

// The constructor.
BasicTransaction::BasicTransaction(RemoteData* rdata, const Range& range, unsigned attempts, unsigned wait)
    : m_range(range),
    mptr_rdata(rdata),
    mptr_thread(NULL),
    mptr_speedThread(NULL),
    m_state(State::idle),
    m_bytesDone(0),
    m_connAttempts(attempts),
    m_attemptWait(wait),
    m_pauseRequest(false)
{

    mptr_response = new boost::asio::streambuf;
    // If the range has a zero size but is not an unitialized range,
    // the download is immediately complete
    if (!m_range.uninitialized() && m_range.size()==0)
        m_state = State::complete;
}

// Static factory methods for generating objects of this type.
// They can be provided with a RemoteData object or a url,
// and an optional byterange.
BasicTransaction* BasicTransaction::factory(RemoteData* rdata,
        const Range& range, unsigned attempts, unsigned wait) {

    if (!rdata) // Danger!! Null pointer
        Throw(ex::Invalid, "The RemoteData pointer passed");

    BasicTransaction* product;  // The product of our factory
    switch (rdata->scheme()) {
        case RemoteData::Protocol::http:
            product = new HttpTransaction<PlainSock>(
                    dynamic_cast<RemoteDataHttp*>(rdata),range,
                    attempts, wait);
            break;
        case RemoteData::Protocol::https:
            product = new HttpTransaction<SSLSock>(
                    dynamic_cast<RemoteDataHttp*>(rdata),range,
                    attempts, wait);
            break;
        default:
            Throw(ex::download::BadProtocolScheme,rdata->schemeCStr());
    }
    return product;
}

BasicTransaction* BasicTransaction::factory(std::string url,
        const Range& range, unsigned attempts, unsigned wait) {
    RemoteData* rdata_url = RemoteData::factory(url);
    return factory(rdata_url, range, attempts, wait);
}

// Register the byte-Reader callback function
// The parameter must be a boost::function object of type
// void() (std::istream&, uintmax_t), which is a void function
// with two parameters : an istream to read from an the number
// of bytes to read.
void BasicTransaction::registerReader(
        boost::function<void (std::istream&, uintmax_t,uintmax_t )>& bytereader){
    m_reader = bytereader;
}

// Clone this BasicTransaction object. A range has to be
// provided and a new socket may be passed in case you want
// it to be used.
BasicTransaction* BasicTransaction::clone(const Range& r, PlainSock* sock) {
    BasicTransaction* bt = factory(mptr_rdata,r);
    bt->injectSocket(sock);
    return bt;
}

BasicTransaction* BasicTransaction::clone(const Range& r, SSLSock* sock) {
    BasicTransaction* bt = factory(mptr_rdata,r);
    bt->injectSocket(sock);
    return bt;
}

// Pause the download. This pause is just a temporary hold,
// used by the Aggregator class to prevent writeouts when
// in the process of splitting. To actually *pause* the
// download, stop() is used. The resuming is handled by
// Aggregator.
void BasicTransaction::pause() {
    m_pauseRequest = true;
}

// Resume after a previous pause() call.
void BasicTransaction::play() {
    m_pauseRequest = false;
}

// Block until the download is finished or fails.
void BasicTransaction::join() const {
    if(mptr_thread)
        mptr_thread->join();
    if(mptr_speedThread)
        mptr_speedThread->join();
}


// Getters and setters for the data members.

// Get state as BasicTransaction::State
typename BasicTransaction::State
BasicTransaction::state() const {
    return m_state;
}


// and as a string
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

// Returns if download is started (success or failure)
bool BasicTransaction::isDownloading() const {
    return state()==State::downloading;
}

// Returns if download is started (success or failure)
bool BasicTransaction::isRunning() const {
    return state()!=State::idle && !isComplete();
}

// Returns if complete (success or failure)
bool BasicTransaction::isComplete() const {
    return state()==State::complete;
}

bool BasicTransaction::hasFailed() const {
    return state()==State::failed;
}

// Get the byterange
const Range& BasicTransaction::range() const {
    return m_range;
}

Range& BasicTransaction::range() {
    return m_range;
}

// Get the remotedata pointer
const RemoteData& BasicTransaction::remoteData() const {
    return *mptr_rdata;
}

// Get a pointer to the downloader thread object
boost::thread* BasicTransaction::p_thread() const {
    return mptr_thread;
}

// Get a pointer to the speed observer thread object
boost::thread* BasicTransaction::p_speedThread() const {
    return mptr_speedThread;
}

// Return the writeout callback functor
boost::function<void (std::istream&, uintmax_t,uintmax_t)>
BasicTransaction::reader() const {
    return m_reader;
}

// Returns the endpoint iterator
tcp::resolver::iterator BasicTransaction::endpIterator() const {
    return m_endpIterator;
}

// Return the bytes done, thats the number of bytes downloaded
// and actually written out.
uintmax_t BasicTransaction::bytesDone() const {
    return m_bytesDone;
}

// return the total number of bytes to be downloaded. Initially this
// is the size indicated by m_range, but will be different if
// the server sends a differently sized response
uintmax_t BasicTransaction::bytesTotal() const {
    return m_range.size();
    //return m_bytesTotal;
}

// total-done
uintmax_t BasicTransaction::bytesRemaining() const {
    return bytesTotal()-m_bytesDone;
    //return m_bytesTotal-m_bytesDone;
}

// Speed observation functions

// The average speed, in bytes per second. Average speed is
// bytesDone/timeElapsed
double BasicTransaction::avgSpeed() const {
    return m_avgSpeed;
}

// The 'instantaneous' speed in bytes per second. It is calculated
// for very short intervals of time.
double BasicTransaction::instSpeed() const {
    return m_instSpeed;
}

// The 'hifi' speed, which I don't understand properly.
double BasicTransaction::hifiSpeed() const {
    return m_hifiSpeed;
}

// THE speed, which will be shown at the frontend.
double BasicTransaction::speed() const {
    return hifiSpeed();
}

// An estimate of the time remaining, in seconds.
uintmax_t BasicTransaction::timeRemaining() const {
    if (speed()==0)
        return std::numeric_limits<uintmax_t>::max();
    return bytesRemaining()/speed();
}

// The function that runs on m_speedThread and does the speed
// calculation work.
void BasicTransaction::speedWorker() {
    /*
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
       */
    const double refresh = 0.1;
    const unsigned persistance = 1/refresh;
    uintmax_t no = 0;
    while( state() != State::downloading && state() !=State::complete)
        boost::this_thread::sleep(boost::posix_time::millisec(refresh*1000));
    while (state() != State::complete) {
        uintmax_t bytes_downloaded = bytesDone();
        boost::this_thread::sleep(boost::posix_time::millisec(refresh*1000));
        uintmax_t delta = bytesDone() - bytes_downloaded;

        m_instSpeed = delta/refresh;
        m_avgSpeed= (m_avgSpeed*no+m_instSpeed)/(no+1);
        int per = (no < persistance)?no:persistance;
        m_hifiSpeed= (m_hifiSpeed*per+m_instSpeed)/(per+1);
        no++;
    }
    m_instSpeed = 0;
    m_avgSpeed = 0;
    m_hifiSpeed = 0;
}

void BasicTransaction::exbridge(ExBridge* exb) {
    mptr_exbridge = exb;
}

ExBridge* BasicTransaction::exbridge() const {
    return mptr_exbridge;
}

// End file BasicTransaction.cpp
