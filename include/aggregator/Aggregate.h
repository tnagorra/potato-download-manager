#ifndef __CO_AGGREGATE__
#define __CO_AGGREGATE__

#include <limits>
#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include "transaction/RemoteData.h"
#include "transaction/Transaction.h"
#include "filesystem/File.h"
#include "filesystem/Directory.h"
#include "aggregator/Chunk.h"
#include "aggregator/ex.h"
#include "common/helper.h"
#include "common/ExBridge.h"
#include "options/LocalOptions.h"

class Aggregate{
    private:
        // TODO
        // store usable sockets for reuse
        //std::vector<Socket*> m_free_socket;

        // the working thread
        boost::thread* mptr_thread;
        // the speed managing thread
        boost::thread* mptr_speed_thread;
        // vector to save the chunks
        std::vector<Chunk*> m_chunk;

        // the url of the file
        std::string m_url;
        // the hash of url
        std::string m_purgatory_filename;
        // the proper filename from url
        std::string m_destination_filename;
        // maximum number of Chunks

        unsigned m_chunks;
        // maximum number of inactive Chunks
        unsigned m_inactive_chunks;
        // Minimum size to be splittable
        uintmax_t m_splittable_size;
        // Size of the download file
        uintmax_t m_filesize;

        // Gives the average speed
        double m_avgSpeed;
        // Gives the instantaneous speed
        double m_instSpeed;
        // Gives the hifi speed
        double m_hifiSpeed;

        // The ExBridge object to pass to transactions
        ExBridge* m_txnExBridge;
        // Failed or not, exceptions can't be thrown outside the worker
        bool m_failed;

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
            if(mptr_thread)
                mptr_thread->join();
            if(mptr_speed_thread)
                mptr_speed_thread->join();
        }

        // Create a thread to start Chunk
        void start() {
            // if mptr_thread represents a thread of execution
            // then don't start the thread again
            //if( mptr_thread.get_id() == boost::thread::id() )
            if (!mptr_thread || !mptr_thread->joinable())
                mptr_thread = new boost::thread(&Aggregate::worker,this);

            if (!mptr_speed_thread || !mptr_speed_thread->joinable()){
                mptr_speed_thread = new boost::thread(&Aggregate::speed_worker,this);
            }
        }

        // Stop the downloads
        void stop();

        // Displays cool information about stuffs
        // and returns no. of lines showed.
        unsigned displayChunks() const;

        void displayExceptions();

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

        // Returns a pretty name
        std::string destinationFilename() const {
            return m_destination_filename;
        }

        // Returns the hashed name
        std::string purgatoryFilename() const {
            return m_purgatory_filename;
        }

        // Returns name of the Chunk with starting byte num
        std::string chunkFilename(uintmax_t num) const {
            return m_purgatory_filename+"/"+std::to_string(num);
        }

        // Returns the total time Remaining
        uintmax_t timeRemaining() const {
            // 0.1 is the precission limit
            if(speed() < 0.99 )
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

        // Throws exception if any of the RemoteTransaction
        // has failed.
        void checkValidity() const;
};

#endif
