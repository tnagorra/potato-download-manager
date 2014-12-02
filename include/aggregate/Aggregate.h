#ifndef __CO_AGGREGATE__
#define __CO_AGGREGATE__

#include<iostream>
#include<vector>
#include<boost/date_time/posix_time/posix_time.hpp>
#include<boost/thread.h>
#include"transaction/Transaction.h"
#include"filesystem/File.h"
#include"filesystem/Directory.h"
#include"aggregate/Chunk.h"


class Aggregate{
    private:
        boost::thread m_thread;
        std::vector<Socket*> m_free_socket;
        std::vector<Chunk*> m_chunk;
        // the url of the file
        string m_url;
        // the hash of url
        string m_hasedUrl;
        // the proper filename from url
        string m_prettyUrl;
        // maximum number of Chunks
        unsigned m_chunks;
        // Minimum size to be splittable
        uintmax_t m_splittable_size;
        // Size of the download file
        uintmax_t m_filesize;
    public:
        // Constructor
        Aggregate(const std::string url, unsigned txns=8,uintmax_t split=10*1024);

        // Destructor
        ~Aggregate();

        // Join all the Txn threads of Chunks in vector
        void join();

        // Stop the downloads
        void stop();

        // Create a thread to start Chunk
        void start();

        // Total data downloaded; includes already saved file
        uintmax_t bytesDone() const;

        // Returns name of the Chunk with starting byte num
        std::string chunkName(uintmax_t num) const;

        // Returns a pretty name
        std::string prettyName() const;

        // Returns the number of active Txns in the vector
        // active implies it has been started but isn't complete yet
        unsigned activeChunks() const;

        // Returns if all the Chunk in the vector are complete
        bool isComplete() const;

        // Returns if any of the Chunk is splittable
        bool isSplittable() const;

        // Returns if all the Txns are downloading something
        bool isSplitReady() const;

        // Returns the index of bottleneck Chunk
        std::vector<Chunk*>::size_type bottleNeck() const;

        // Split a Chunk and insert new Chunk after it
        void split(std::vector<Chunk*>::size_type split_index);

    private:
        // Runs after start() in a thread
        void worker();

        // Merge all the Chunks
        void merger();

        // Monitors chunk for splitting
        void splitter();

        // Blocks the flow until all Chunks are complete
        void blocker();

        // Starts the process, finds about previous sessions
        void starter();
};

#endif
