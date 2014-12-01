#include"aggregate/Chunk.h"

Chunk::Chunk(Transaction* txn, File* file):
    mptr_txn(txn), mptr_file(file)
{
    if(mptr_file == NULL)
        Throw(ex::Invalid,"File");
    if(mptr_txn == NULL)
        Throw(ex::Invalid,"Transaction");

    // Initialize m_fileSize with size of file
    m_fileSize = m_file->size();

    // Bind and inject the append function of File
    // inside the Transaction
    boost::function<void (std::istream&,uintmax_t)> reader = boost::bind(
            static_cast<void(File::*)(std::istream&,uintmax_t)>(&File::append),
            mptr_file,_1,_2);
    mptr_txn->registerReader(reader);
}

Chunk::~Chunk() {
    // Delete the file and transaction
    // assosicated with it
    if(mptr_txn)
        delete *mptr_txn;
    if(mptr_file)
        delete *mptr_file;
}

// Return pointer to transaction
Transaction* const Chunk::txn() {
    return m_txn;
}

// Return pointer to file
File* const Chunk::file() {
    return m_file;
}

// Return total downloaded bytes of a Chunk
// Includes data bytes saved + data bytes downloaded
uintmax_t Chunk::bytesDown() const {
    return m_fileSize + mptr_txn->bytesDown();
}
