#ifndef __CO_AGGREGATE__
#define __CO_AGGREGATE__

#include <limits>
#include <iostream>
#include <string>
#include <vector>
#include "transaction/RemoteData.h"
#include "transaction/Transaction.h"
#include "filesystem/File.h"
#include "filesystem/Directory.h"
#include "aggregator/Chunk.h"
#include "aggregator/ex.h"
#include "common/helper.h"
#include "common/ExBridge.h"

/*
   When BasicTransaction is complete, push those to m_socket
   When BasicTransaction is started and m_socket isn't empty,
   take it from m_socket and pop it
   if it is empty, create new inside the BasicTransaction
   */

class Aggregate{
    private:
        // store usable sockets for reuse
        //std::vector<Socket*> m_free_socket;
        // the working thread
        boost::thread* m_thread;
        // the speed managing thread
        boost::thread* m_speed_thread;
        // vector to save the chunks
        std::vector<Chunk*> m_chunk;
        // the url of the file
        std::string m_url;
        // the hash of url
        std::string m_hashedUrl;
        // the proper filename from url
        std::string m_prettyUrl;
        // maximum number of Chunks
        unsigned m_chunks;
        // maximum number of inactive Chunks
        unsigned m_inactive_chunks;
        // Minimum size to be splittable
        uintmax_t m_splittable_size;
        // Size of the download file
        uintmax_t m_filesize;
        // Failed or not, exceptions can't be
        // thrown outside the worker
        bool m_failed;
        // Gives the average speed
        double m_avgSpeed;
        // Gives the instantaneous speed
        double m_instSpeed;
        // Gives the hifi speed
        double m_hifiSpeed;
        // The ExBridge object to pass to transactions
        ExBridge* m_txnExBridge;

    public:
        // Constructor
        Aggregate(const std::string& url,
                const std::string& destination,
                const std::string& purgatory,
                unsigned txns=8,uintmax_t split=500*1024);

        // Destructor
        ~Aggregate();

        // Join it's thread
        void join(){
            // If the thread has completed or represents Not-A-Thread
            // it just returns
            if(m_thread)
                m_thread->join();
            if(m_speed_thread)
                m_speed_thread->join();
        }

        // Stop the downloads
        void stop();

        // Create a thread to start Chunk
        void start() {
            // if m_thread represents a thread of execution
            // then don't start the thread again
            //if( m_thread.get_id() == boost::thread::id() )
            if (!m_thread || !m_thread->joinable())
                m_thread = new boost::thread(&Aggregate::worker,this);

            if (!m_speed_thread || !m_speed_thread->joinable()){
                m_speed_thread = new boost::thread(&Aggregate::speed_worker,this);
            }
        }

        // Displays cool information about stuffs
        unsigned displayChunks() const;

        // Total data downloaded; includes already saved bytes
        uintmax_t bytesDone() const;

        // Total data that we are trying to download
        uintmax_t bytesTotal() const;

        // Returns the total progress
        double progress() const {
            if( m_filesize == 0 ) return 0;
            return 1.0*bytesDone()/m_filesize*100;
        }

        // Returns if all the Chunk in the vector are complete
        bool isComplete() const;

        // Returns if the transactions are finished
        // ie, either failed or complete
        bool isFinished() const;

        // Return if the transaction has failed or not
        inline bool hasFailed() const {
            return m_failed;
        }

        // The average speed, in bytes per second. Average speed is
        // bytesDone/timeElapsed
        double avgSpeed() const {
            return m_avgSpeed;
        }

        // The 'instantaneous' speed in bytes per second. It is calculated
        // for very short intervals of time.
        double instSpeed() const {
            return m_instSpeed;
        }

        // The 'hifi' speed, which I don't understand properly.
        double hifiSpeed() const {
            return m_hifiSpeed;
        }

        // THE speed, which will be shown at the frontend.
        double speed() const {
            return instSpeed();
        }

        // Returns the total time Remaining
        uintmax_t timeRemaining() const {
            // .99 is the precission limit
            if(speed() <= .99 )
                return std::numeric_limits<uintmax_t>::max();
            return bytesTotal() > bytesDone() ? (bytesTotal() - bytesDone()) / speed() : 0;
        }

        // Returns the number of active BasicTransactions in the vector
        // active implies it has been started but isn't complete yet
        unsigned activeChunks() const ;

        // Returns the number of total BasicTransactions in the vector
        unsigned totalChunks() const {
            return m_chunk.size();
        }

    private:
        // Gets the sum of all speed
        double aggregateSpeed() const;

        // Runs after start() in a thread
        void worker();

        void speed_worker();

        // Merge all the Chunks
        void merger();

        // Monitors chunk for splitting
        void splitter();

        // Starts the process, finds about previous sessions
        void starter();

        // Join all the BasicTransaction threads of Chunks in vector
        void joinChunks();

        // Returns if all the BasicTransactions are downloading something
        bool isSplitReady() const;

        // Returns the index of bottleneck Chunk
        std::vector<Chunk*>::size_type bottleNeck() const;

        // Split a Chunk and insert new Chunk after it
        void split(std::vector<Chunk*>::size_type split_index);

        // Returns name of the Chunk with starting byte num
        std::string chunkName(uintmax_t num) {
            return m_hashedUrl+"/"+std::to_string(num);
        }

        // Returns a pretty name
        std::string prettyName() {
            return m_prettyUrl;
        }
};

#endif
