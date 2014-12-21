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

/*
   TODO
    1. Reusing sockets
       When BasicTransaction is complete, push those to m_socket
       When BasicTransaction is started and m_socket isn't empty,
       take it from m_socket and pop it
       if it is empty, create new inside the BasicTransaction
    2. Implement write/read lock
    3. Catch exceptions inside BasicTransaction
    4. Don't use beenSplit and seperate bytesTotal
    4. Something in updateRange()
    5. Consider what to do when server doesn't provide size info
    7. Use Option in Aggregate class
    8. Make Options give data outside
*/

class Aggregate{
    private:
        // store usable sockets for reuse
        //std::vector<Socket*> m_free_socket;
        // the working thread
        boost::thread* m_thread;
        // vector to save the chunks
        std::vector<Chunk*> m_chunk;
        // the save folder of the file
        std::string m_heaven;
        // the temporary folder of the file
        std::string m_purgatory;
        // the url of the file
        std::string m_url;
        // the hash of url
        std::string m_hashedUrl;
        // the proper filename from url
        std::string m_prettyUrl;
        // maximum number of Chunks
        unsigned m_chunks;
        // Minimum size to be splittable
        uintmax_t m_splittable_size;
        // Size of the download file
        uintmax_t m_filesize;
        // Failed or not, exceptions can't be
        // thrown outside the worker
        bool m_failed;
    public:
        // Constructor
        Aggregate(const std::string& url,
                const std::string& heaven="potatoes",
                const std::string& purgatory="potas",
                unsigned txns=8,uintmax_t split=500*1024);

        // Destructor
        ~Aggregate();

        // Join it's thread
        void join(){
            // If the thread has completed or represents Not-A-Thread
            // it just returns
            if(m_thread)
                m_thread->join();
        }

        // Stop the downloads
        void stop();

        // Create a thread to start Chunk
        void start() {
            // if m_thread represents a thread of execution
            // then don't start the thread again
            //if( m_thread.get_id() == boost::thread::id() )
            if (m_thread && m_thread->joinable())
                return;
            m_thread = new boost::thread(&Aggregate::worker,this);
        }

        // Displays cool information about stuffs
        unsigned display();

        // Display progress bar
        void progressbar();

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

        // Returns the total speed of the Chunks
        double speed() const;

        // Returns the total time Remaining
        uintmax_t timeRemaining() const {
            if(speed()== 0)
                return std::numeric_limits<uintmax_t>::max();
            return bytesTotal() > bytesDone() ? (bytesTotal() - bytesDone()) / speed() : 0;
        }

        // Returns the number of active BasicTransactions in the vector
        // active implies it has been started but isn't complete yet
        unsigned activeChunks() const;

        // Returns the number of total BasicTransactions in the vector
        inline unsigned totalChunks() const {
            return m_chunk.size();
        }

        inline bool hasFailed() const {
            return m_failed;
        }
    private:
        // Runs after start() in a thread
        void worker();

        // Merge all the Chunks
        void merger();

        // Monitors chunk for splitting
        void splitter();

        // Starts the process, finds about previous sessions
        void starter();

        // Join all the BasicTransaction threads of Chunks in vector
        void joinChunks();

        // Returns if any of the Chunk is splittable
        RemoteData::Partial isSplittable() const;

        // Returns if all the BasicTransactions are downloading something
        bool isSplitReady() const;

        // Returns the index of bottleneck Chunk
        std::vector<Chunk*>::size_type bottleNeck() const;

        // Split a Chunk and insert new Chunk after it
        void split(std::vector<Chunk*>::size_type split_index);

        // Returns name of the Chunk with starting byte num
        inline std::string chunkName(uintmax_t num) const {
            // Choice of "/" or "\" is taken care of inside class File
            return m_hashedUrl+"/"+std::to_string(num);
        }

        // Returns a pretty name
        inline std::string prettyName() const {
            return m_prettyUrl;
        }
};

#endif
