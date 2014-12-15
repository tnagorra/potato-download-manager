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

        // Returns reference of BasicTransaction
        inline BasicTransaction& txn(){
            return *mptr_txn;
        }

        // Returns const reference of BasicTransaction
        inline const BasicTransaction& txn() const {
            return *mptr_txn;
        }

        // Returns reference of File
        inline File& file(){
            return *mptr_file;
        }

        // Returns const reference of File
        inline const File& file() const {
            return *mptr_file;
        }

        // Return total downloaded bytes of a Chunk
        // Includes data bytes saved + data bytes downloaded
        inline uintmax_t bytesDone() const {
            return m_fileSize + txn().bytesDone();
        }

        // Return total bytes of a Chunk
        inline uintmax_t bytesTotal() const {
            return m_fileSize + txn().bytesTotal();
        }
};

#endif
