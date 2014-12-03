#include"aggregate/Chunk.h"

Chunk::Chunk(BasicTransaction* txn, File* file):
    mptr_txn(txn), mptr_file(file)
{
    if(mptr_file == NULL)
        Throw(ex::Invalid,"File");
    if(mptr_txn == NULL)
        Throw(ex::Invalid,"BasicTransaction");

    // Initialize m_fileSize with size of file
    m_fileSize = mptr_file->size();

    // Bind and inject the append function of File
    // inside the BasicTransaction
    boost::function<void (std::istream&,uintmax_t)> reader = boost::bind(
            static_cast<void(File::*)(std::istream&,uintmax_t)>(&File::append),
            mptr_file,_1,_2);
    mptr_txn->registerReader(reader);
}

Chunk::~Chunk() {
    // Delete the file and transaction
    // assosicated with it
    if(mptr_txn)
        delete mptr_txn;
    if(mptr_file)
        delete mptr_file;
}

BasicTransaction* const Chunk::txn() {
    return mptr_txn;
}

File* const Chunk::file() {
    return mptr_file;
}

uintmax_t Chunk::bytesDone() const {
    return m_fileSize + mptr_txn->bytesDone();
}

uintmax_t Chunk::bytesTotal() const {
    return m_fileSize + mptr_txn->bytesTotal();
}
