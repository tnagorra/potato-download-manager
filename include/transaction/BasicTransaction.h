// File BasicTransaction.h
// Contains the class BasicTransaction

#ifndef __CO_BASICTRANSACTION__
#define __CO_BASICTRANSACTION__

#include <string>
#include <istream>
#include <limits>

#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/function.hpp>

#include "common/ex.h"
#include "transaction/ex.h"
#include "transaction/RemoteData.h"
#include "transaction/Range.h"
#include "transaction/SocketTypes.h"

using boost::asio::ip::tcp;

// class BasicTransaction
// This class is the core of the downloader, in the sense that the
// actual downloading happens and is co-ordinated here.
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
        // function object for writing out the received bytes
        boost::function<void (std::istream&, uintmax_t)> m_reader;
        // Endpoint iterator for hostname resolution
        tcp::resolver::iterator m_endpIterator;
        // Has the transaction been split externally?
        bool m_beenSplit;
        // Has the transaction just been paused?
        bool m_beenPaused;

        // Total number of bytes to be downloaded
        uintmax_t m_bytesTotal;
        // Total number of bytes downloaded
        uintmax_t m_bytesDone;

        // Timer thread variables. These are only to be mutated by
        // the timer thread.
        double m_avgSpeed;
        double m_instSpeed;
        double m_hifiSpeed;
    public:
        // Constructor. Default Range argument (0,0) means the
        // entire resource
        BasicTransaction(RemoteData* rdata,
                Range range = Range(0,0));

        // Static factory method for generating objects of this
        // type.
        // TODO should we encapsulate the factory in another class?
        static BasicTransaction* factory(RemoteData* rdata,
                Range range = Range(0,0));

        static BasicTransaction* factory(std::string url,
                Range range = Range(0,0));

        // Destructor
        virtual ~BasicTransaction() {}

        // Returns if complete
        bool isComplete() const;

        // Start the download
        virtual void start()=0;

        // Stop the download
        virtual void stop()=0;

        // Register the byte-Reader function
        void registerReader(boost::function<void
                (std::istream&, uintmax_t)>&);

        // Returns the current state as state
        State state() const;

        // Returns the current state as string
        std::string stateString() const;

        Range range() const;

        RemoteData* p_remoteData() const;

        boost::thread* p_thread() const;

        boost::thread* p_speedThread() const;

        boost::function<void (std::istream&,uintmax_t)> reader() const;

        tcp::resolver::iterator endpIterator() const;

        uintmax_t bytesTotal() const;

        uintmax_t bytesDone() const;

        uintmax_t bytesRemaining() const;

        bool isRunning() const;

        void pause();

        bool isPaused() const;

        void play();

        virtual void injectSocket(SSLSock* sock)=0;

        virtual void injectSocket(PlainSock* sock)=0;

        BasicTransaction* clone(Range r, PlainSock* sock=NULL);

        BasicTransaction* clone(Range r, SSLSock* sock);

        void join() const;

        // Update the upper byte of the byte range
        void updateRange(uintmax_t u);

        // Getters for getting speed related values
        double avgSpeed() const;
        double instSpeed() const;
        double hifiSpeed() const;
        double speed() const;
        uintmax_t timeRemaining() const;
    protected:
        void speedWorker();
};
#endif
// End File BasicTransaction.h
