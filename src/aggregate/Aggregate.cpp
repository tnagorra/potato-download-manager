#include"aggregate/Aggregate.h"

/*
   TODO 1
   Join transaction thread somewhere

   When Txn is complete, push those to m_socket
   When Txn is started and m_socket isn't empty,
   take it from m_socket and pop it
   if it is empty, create new inside the Txn
*/

Aggregate::Aggregate(const std::string url, unsigned txns,uintmax_t split):
    m_chunks(txns), m_url(url), m_splittable_size(split), m_filesize(0)
{
    m_hasedUrl = md5(m_url);
    m_prettyUrl = prettify(m_url);
}

Aggregate::~Aggregate(){
    // Delete all Chunk and Socket
    for (auto it = m_chunk.begin(); it != m_chunk.end(); ++it)
        delete *it;
    for (auto it = m_free_socket.begin(); it != m_free_socket.end(); ++it)
        delete *it;
}

void Aggregate::join(){
    // Join all Chunks
    for(auto it = m_chunk.begin(); it != m_chunk.end(); ++it)
        it->txn()->join();
}

void Aggregate::stop(){
    /* TODO stop everything
       for(auto it = m_chunk.begin(); it != m_chunk.end(); ++it){
       it->txn()->stop();
       }
       */
}

void Aggregate::start() {
    m_thread = boost::thread(&Chunk::worker,this);
}

uintmax_t Aggregate::bytesDown() const {
    uintmax_t bytes = 0;
    for (auto it = m_chunk.begin(); it != m_chunk.end(); ++it)
        bytes += it->size();
    return bytes;
}

std::string Aggregate::chunkName(uintmax_t num) const {
    // NOTE: "/" or "\" doesn't matter
    return m_hasedUrl+"/"+std::to_string(num);
}

std::string Aggregate::prettyName() const {
    return m_prettyUrl;
}

unsigned Aggregate::activeChunks() const {
    unsigned count = 0;
    for (auto it = m_chunk.begin(); it != m_chunk.end(); ++it){
        if (it->txn()->isRunning())
            count++;
    }
    return count;
}

bool Aggregate::isComplete() const {
    for (auto it = m_chunk.begin(); it != m_chunk.end(); ++it){
        if(!it->txn()->isComplete())
            return false;
    }
    return true;
}

bool Aggregate::isSplittable() const {
    for (auto it = m_chunk.begin(); it != m_chunk.end(); ++it){
        if(!it->txn()->bytesRemaining() > m_splittable_size)
            return true;
    }
    return false;
}

bool Aggregate::isSplitReady() const {
    for (auto it = m_chunk.begin(); it != m_chunk.end(); ++it){
        // TODO 0 Insert some really good algorithm
        // Maybe wait for download to get to stable state
        // Waiting for a while may be benificial in long run
        if (it->txn()->speed() <= 100 && !it->tx()->isComplete())
            return false;
    }
    return true;
}

std::vector<Chunk*>::size_type Aggregate::bottleNeck() const {
    // Initialize bottleneck such that it is the first chunk
    // which is splittable
    uintmax_t bbr = 0;
    uintmax_t btr = 0;
    std::vector<Chunk*>::size_type bneck = 0;
    std::vector<Chunk*>::size_type it = 0;
    for (;it < m_chunk.size();++it){
        uintmax_t ibr = m_chunk[it]->txn()->bytesRemaining();
        if(ibr <= m_splittable_size)
            continue;
        // If it is splittable, it is the bottleneck
        bneck = it;
        bbr = m_chunk[bneck]->txn()->bytesRemaining();
        btr = m_chunk[bneck]->txn()->timeRemaining();
        break;
    }

    // If there is no bottleneck Chunk then throw exception
    if( it == m_chunk.size() )
        Throw(ex::chunk::NoBottleNecK());

    // Now just get the real bottle neck
    for (; it < m_chunk.size(); ++it){
        uintmax_t ibr = m_chunk[it]->txn()->bytesRemaining();
        if( ibr <= m_splittable_size)
            continue;
        uintmax_t itr = m_chunk[it]->txn()->timeRemaining();
        if((btr < itr) || (btr==itr && bbr<ibr)){
            bneck = itr;
            bbr = m_chunk[bneck]->txn()->bytesRemaining();
            btr = m_chunk[bneck]->txn()->timeRemaining();
        }
    }
    return bneck;
}

// Split a Chunk and insert new Chunk after it
void Aggregate::split(std::vector<Chunk*>::size_type split_index){
    // Get the Chunk to be splitted
    // Pause the Txn
    // Wait until it is paused
    Chunk* cell = m_chunk[split_index];
    cell->txn()->pause();
    while(!cell->txn()->isPaused())
        boost::this_thread::sleep(boost::posix_time::millisec(100));

    // Calculate the split point ie midpoint
    // Note: Range is exclusive of previous session
    uintmax_t downloaded = cell->txn()->downBytes();
    uintmax_t lower = cell->txn()->range()->lower();
    uintmax_t upper = cell->txn()->range()->upper();
    uintmax_t midpoint = (upper+(lower+downloaded))/2;

    // Create a new cloned Txn instance and update values
    // Create a new File and Chunk objects
    File* newfile = new File(chunkName(midpoint));
    cell->txn()->range()->update(midpoint,lower);
    Txn* newtxn = cell->txn()->clone(upper,midpoint);
    Chunk* newcell = new Chunk(newtxn,newfile);

    // Insert newly created Chunk in the vector
    m_chunk.insert(++splint_index, newcell);

    // Start those Txns
    cell->txn()->start();
    newcell->txn()->start();
}

void Aggregate::worker(){
    starter();
    splitter();
    blocker();
    merger()
}

void Aggregate::merger() {
    // If total size downloaded isn't equal
    // to the size of file downloaded then
    // do not merge the Chunks
    if( size() != m_filesize )
        throw(ex::Invalid,"filesize");

    // Create a file where chunk files are merged
    File merged(prettyName());
    // Append the content to "merged" and remove
    // the chunk files
    for(auto it = m_chunk.begin(); it != m_chunk.end(); ++it){
        merged.append(it->file());
        it->file()->remove();
    }
    // Remove the old directory
    Directory(m_hasedUrl).remove();
}

void Aggregate::splitter() {
    // Loop while a bottleneck exists
    while (true){
        // Sleep
        boost::this_thread::sleep(boost::posix_time::millisec(100));

        // Get the bottle neck and split
        // NOTE: There may be some problem here
        if(activeChunks() < m_chunks && isSplitReady()){
            try
                std::vector<Chunk*>::size_type bneck = bottleNeck();
            catch (ex::chunk::NoBottleNeck)
                break;
            split(bneck);
        }
    }
}

void Aggregate::blocker() {
    // loop while every chunk is complete
    while (!isComplete()){
        boost::this_thread::sleep(boost::posix_time::millisec(100));
    }
}

void Aggregate::starter() {

    // Directory session is used to find out about
    // previous downloads
    Directory session(m_hasedUrl);
    if ( session.exist() && !session.isEmpty() ) {

        // Get the list of files, exclude the last one
        // Iterate over them to find the first chunk
        // which isn't complete yet
        // Start it and save the name
        // After downlaod starts
        //
        // We have information about the type
        // Now clone from it and update appropriate
        // values to start all

        // TODO maybe create an empty file to save the
        // total size of the downloading file but don't
        // don't push it in the Chunk for download


        // Set m_filesize by looking at the last thing
    } else {
        // Researcher finds about the necessary information
        // about the download file; filesize, resumability
        // NOTE: No range is sent to the Txn
        File* newfile = new File(chunkName(0));
        Txn* newtxn = TxnFactory::getInstance(url);
        Chunk* researcher = new Chunk(newtxn,newfile);
        m_chunk.push(researcher);
        researcher->txn->start();

        // Wait for researcher until downloading starts,
        // Now we get the proper information about the file
        // and further process can be started
        while (reasearcher->txn->state() != BasicTransaction::State::downloading)
            boost::this_thread::sleep(boost::posix_time::millisec(100));

        // Initialize m_filesize
        m_filesize = researcher->txn()->bytesTotal();
    }
}
