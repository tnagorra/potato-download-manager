#ifndef __CO_CHUNK__
#define __CO_CHUNK__

#include <iostream>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include "common/ex.h"
#include "filesystem/File.h"
#include "filesystem/Directory.h"
#include "transaction/BasicTransaction.h"

// Chunk is wrapper of a BasicTransaction and a File associated with it
class Chunk {
    private:
        // Initial filesize when opening
        uintmax_t m_fileSize;
        BasicTransaction* mptr_txn;
        File* mptr_file;

    public:
        // Constructor with injection
        Chunk(BasicTransaction* txn, File* file);

        // Destructor
        ~Chunk();

        // Returns pointer of BasicTransaction
        BasicTransaction* const txn();

        // Returns pointer to File
        File* const file();

        // Return total downloaded bytes of a Chunk
        // Includes data bytes saved + data bytes downloaded
        uintmax_t bytesDone() const;

        // Return total bytes of a Chunk
        uintmax_t bytesTotal() const;
};

#endif
