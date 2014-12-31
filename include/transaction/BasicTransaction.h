// File BasicTransaction.h
// Contains the class BasicTransaction

#ifndef __CO_BASICTRANSACTION__
#define __CO_BASICTRANSACTION__

#include <string>
#include <istream>
#include <limits>

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/asio.hpp>
#include <boost/function.hpp>

#include "common/ex.h"
#include "transaction/ex.h"
#include "transaction/RemoteData.h"
#include "transaction/Range.h"
#include "transaction/SocketTypes.h"


#include "filesystem/File.h"
using boost::asio::ip::tcp;

// class BasicTransaction (abstract)
// This class is the core of the download manager, in the sense that
// the actual downloading happens and is co-ordinated here. This class
// is also the sole interface that is used by other components of the
// system for getting bytes from a remote host.
class BasicTransaction {

    // Public data members
    public:
        enum class State { idle, resolving, connecting, requesting,
            waiting, ready, downloading, complete, failed };

    // Protected data members
    protected:
        // The state which the BasicTransaction is in.
        State m_state;
        // The byte-range of the resource which will be downloaded
        Range m_range;
        // A RemoteData Object pointer
        RemoteData* mptr_rdata;
        // The thread in which the download will run
        boost::thread* mptr_thread;
        // The thread which will do the speed observation work
        boost::thread* mptr_speedThread;
        // Streambuf for buffering the response
        boost::asio::streambuf* mptr_response;
        // function object callback used to write out received bytes
        boost::function<void (std::istream&, uintmax_t, uintmax_t)> m_reader; // Endpoint iterator for hostname resolution
        tcp::resolver::iterator m_endpIterator;
        // Has the transaction been split (range-updated) externally?
        //bool m_beenSplit;
        // Has a pause request been issued?
        bool m_pauseRequest;

        // Total number of bytes to be downloaded. Initially this
        // member is set to the size indicated by m_range, but will
        // be updated if the server sends a differently sized response
        //uintmax_t m_bytesTotal;
        // Total number of bytes that have been downloaded, and
        // actually written out through the callback m_reader()
        uintmax_t m_bytesDone;

        // Timer thread variables. These are only to be mutated by
        // the timer thread.
        double m_avgSpeed;  // Average speed
        double m_instSpeed; // 'Instantaneous' speed
        double m_hifiSpeed; // 'Hifi' speed

    // Public methods (interface)
    public:
        // Constructor. Default Range argument (0,0) means the
        // entire resource
        BasicTransaction(RemoteData* rdata, const Range& range = Range(0,0));

        // Destructor
        virtual ~BasicTransaction() {}

        // Static factory methods for generating objects of this type.
        // They can be provided with a RemoteData object or a url,
        // and an optional byterange.
        static BasicTransaction* factory(RemoteData* rdata,
                const Range& range = Range(0,0));
        static BasicTransaction* factory(std::string url,
                const Range& range = Range(0,0));

        // Register the byte-Reader callback function
        // The parameter must be a boost::function object of type
        // void() (std::istream&, uintmax_t), which is a void function
        // with two parameters : an istream to read from an the number
        // of bytes to read.
        void registerReader(boost::function<void
                (std::istream&, uintmax_t, uintmax_t)>&);

        // Inject a tcp socket to the downloader. These may be called
        // if you have a live socket and want that to be used for the
        // download. Reusing active sockets saves the latency time
        // losses caused when creating new connections. injectSocket
        // may only be called before you start() the download, other-
        // wise it will just return.
        virtual void injectSocket(SSLSock* sock)=0;
        virtual void injectSocket(PlainSock* sock)=0;

        // Clone this BasicTransaction object. A range has to be
        // provided and a new socket may be passed in case you want
        // it to be used.
        BasicTransaction* clone(const Range& r, PlainSock* sock=NULL);
        BasicTransaction* clone(const Range& r, SSLSock* sock);

        // Start the download
        virtual void start()=0;

        // Stop the download
        virtual void stop()=0;

        // Pause the download. This pause is just a temporary hold,
        // used by the Aggregator class to prevent writeouts when
        // in the process of splitting. To actually *pause* the
        // download, stop() is used. The resuming is handled by
        // Aggregator.
        void pause();

        // Resume after a previous pause() call.
        void play();

        // Block until the download is finished or fails.
        void join() const;

        // Getters and setters for the data members.

        // Get state as BasicTransaction::State and as a string
        State state() const;
        std::string stateString() const;

        bool isRunning() const;

        bool isDownloading() const;

        // Returns if complete (success or failure)
        bool isComplete() const;

        // Return if transaction has failed
        bool hasFailed() const;

        // Get the byterange
        const Range& range() const;

        Range& range() ;

        // Get the remotedata reference
        const RemoteData& remoteData() const;

        // Get pointers to the downloader and speed observer threads
        boost::thread* p_thread() const;
        boost::thread* p_speedThread() const;

        // Return the writeout callback functor
        boost::function<void (std::istream&,uintmax_t,uintmax_t)> reader() const;

        // Returns the endpoint iterator
        tcp::resolver::iterator endpIterator() const;

        uintmax_t bytesTotal() const;
        uintmax_t bytesDone() const;
        uintmax_t bytesRemaining() const;

        // Getters for getting speed related values. All values
        // are returned in bytes per second.
        double avgSpeed() const;    // Average speed
        double instSpeed() const;   // 'Instantaneous' speed
        double hifiSpeed() const;   // 'Hifi' speed
        double speed() const;       // THE speed
        // An estimate of the time remaining, in seconds
        uintmax_t timeRemaining() const;

        // Worker function for calculating the speeds
        void speedWorker();
};

#endif
// End File BasicTransaction.h
