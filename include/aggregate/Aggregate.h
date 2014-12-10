#ifndef __CO_AGGREGATE__
#define __CO_AGGREGATE__

#include<iostream>
#include<vector>
#include<boost/date_time/posix_time/posix_time.hpp>
#include<boost/thread.hpp>
#include"transaction/Transaction.h"
#include"filesystem/File.h"
#include"filesystem/Directory.h"
#include"aggregate/Chunk.h"
#include"common/helper.h"
#include"aggregate/ex.h"
#include<string>
#include <limits>

class Aggregate{
    private:
        boost::thread m_thread;
        //std::vector<Socket*> m_free_socket;
        std::vector<Chunk*> m_chunk;
        // the save folder of the file
        std::string m_savefolder;
        // the url of the file
        std::string m_url;
        // the hash of url
        std::string m_hasedUrl;
        // the proper filename from url
        std::string m_prettyUrl;
        // maximum number of Chunks
        unsigned m_chunks;
        // Minimum size to be splittable
        uintmax_t m_splittable_size;
        // Size of the download file
        uintmax_t m_filesize;
    public:
        // Constructor
        Aggregate(const std::string url, const std::string savefolder="Potatoes",unsigned txns=8,uintmax_t split=500*1024);

        // Destructor
        ~Aggregate();

        // Join it's thread
        void join();

        // Stop the downloads
        void stop();

        // Create a thread to start Chunk
        void start();

        // Displays cool information about stuffs
        void display();

        // Display progress bar
        void progressbar();

        // Total data downloaded; includes already saved bytes
        uintmax_t bytesDone() const;

        // Total data that we are trying to download
        uintmax_t bytesTotal() const;

        // Returns the total progress
        double progress() const;

        // Returns if all the Chunk in the vector are complete
        bool complete() const;

        // Returns the total speed of the Chunks
        double speed() const;

        // Returns the total time Remaining
        uintmax_t timeRemaining() const;

        // Returns the number of active BasicTransactions in the vector
        // active implies it has been started but isn't complete yet
        unsigned activeChunks() const;

        // Returns the number of total BasicTransactions in the vector
        unsigned totalChunks() const;

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

        // Returns if all the BasicTransactions are downloading something
        bool splitReady() const;

        // Returns the index of bottleneck Chunk
        std::vector<Chunk*>::size_type bottleNeck() const;

        // Split a Chunk and insert new Chunk after it
        void split(std::vector<Chunk*>::size_type split_index);

        // Returns name of the Chunk with starting byte num
        std::string chunkName(uintmax_t num) const;

        // Returns a pretty name
        std::string prettyName() const;
};

#endif
