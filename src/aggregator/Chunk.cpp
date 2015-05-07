#include"aggregator/Chunk.h"

Chunk::Chunk(BasicTransaction* txnp, File* filep):
    mptr_txn(txnp), mptr_file(filep)
{
    if(mptr_file == NULL)
        Throw(ex::Invalid,"File Pointer");
    if(mptr_txn == NULL)
        Throw(ex::Invalid,"BasicTransaction Pointer");

    // Initialize m_fileSize with size of file
    m_fileSize = file()->size();

    // Bind and inject the append function of File
    // inside the BasicTransaction
    boost::function<void (std::istream&,uintmax_t,uintmax_t)> reader =
        boost::bind(static_cast<void(File::*)(std::istream&,uintmax_t,uintmax_t)>
                (&File::append),mptr_file,_1,_2,_3);
    txn()->registerReader(reader);
}

Chunk::~Chunk() {
    // Delete the file and transaction
    // assosicated with it
    if(mptr_txn != NULL)
        delete mptr_txn;
    if(mptr_file != NULL)
        delete mptr_file;
}
