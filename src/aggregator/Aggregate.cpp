#include"aggregator/Aggregate.h"

Aggregate::Aggregate(const std::string url, const std::string savefolder,
        unsigned txns,uintmax_t split):
    m_chunks(txns), m_url(url), m_splittable_size(split),
    m_filesize(0), m_savefolder(savefolder)
{
    m_hasedUrl = m_savefolder+"/"+md5(m_url);
    m_prettyUrl = m_savefolder+"/"+prettify(m_url);
}

Aggregate::~Aggregate(){
    // Delete all Chunk and Socket
    for (auto it = m_chunk.begin(); it != m_chunk.end(); ++it)
        delete (*it);
    /*
       for (auto it = m_free_socket.begin(); it != m_free_socket.end(); ++it)
       delete *it;
       */
}

void Aggregate::join(){
    // If the thread has completed or represents Not-A-Thread
    // it just returns
    m_thread.join();
}

void Aggregate::stop(){
    // TODO stop everything
}

void Aggregate::start() {
    // if m_thread represents a thread of execution
    // then don't start the thread again
    //if( m_thread.get_id() == boost::thread::id() )
    if (m_thread.joinable())
        return;
    m_thread = boost::thread(&Aggregate::worker,this);
}

void Aggregate::display() {
    fancyprint(activeChunks() << "/" << totalChunks(),NOTIFY);

    for(auto i=0;i<m_chunk.size();i++){
        uintmax_t lower = std::atoi(m_chunk[i]->file()->filename().c_str());
        uintmax_t down= m_chunk[i]->txn()->range().lb()+m_chunk[i]->txn()->bytesDone();
        uintmax_t higher = m_chunk[i]->txn()->range().ub();
        std::string myColor;

        if(m_chunk[i]->txn()->state() == BasicTransaction::State::complete)
            myColor = SUCCESS;
        else if( m_chunk[i]->txn()->state() == BasicTransaction::State::failed)
            myColor = ERROR;
        else if( m_chunk[i]->txn()->state() == BasicTransaction::State::downloading)
            myColor = WARNING;
        else
            myColor = NOTIFY;

        fancyprint(lower << ":" << down << ":"<< higher<< " ",myColor);
    }
}

void Aggregate::progressbar() {
    const unsigned len = 50;
    unsigned place = progress()/100*len;

    fancycout(std::string(place,' '), COLOR(0,CC::WHITE,CC::PURPLE));
    fancycout(std::string(len-place,' '),COLOR(0,CC::PURPLE,CC::WHITE));

    print( " " << round(progress(),2) << "%\t"
            << formatTime(timeRemaining()) << "\t"
            << formatByte(speed()) << "ps\t");
}

uintmax_t Aggregate::bytesDone() const {
    uintmax_t bytes = 0;
    for (auto it = m_chunk.begin(); it != m_chunk.end(); ++it)
        bytes += (*it)->bytesDone();
    return bytes;
}

uintmax_t Aggregate::bytesTotal() const {
    uintmax_t bytes = 0;
    for (auto it = m_chunk.begin(); it != m_chunk.end(); ++it)
        bytes += (*it)->bytesTotal();
    return bytes;
}

bool Aggregate::isComplete() const {
    for (auto it = m_chunk.begin(); it != m_chunk.end(); ++it){
        if( (*it)->txn()->isComplete() == false )
            return false;
    }
    return true;
}

double Aggregate::speed() const {
    double s = 0;
    for(auto i = m_chunk.begin();i != m_chunk.end();++i)
        s += (*i)->txn()->speed();
    return s;
}

unsigned Aggregate::activeChunks() const {
    unsigned count = 0;
    for (auto it = m_chunk.begin(); it != m_chunk.end(); ++it){
        if ((*it)->txn()->isRunning())
            count++;
    }
    return count;
}

void Aggregate::joinChunks(){
    for(auto it = m_chunk.begin(); it != m_chunk.end(); ++it)
        (*it)->txn()->join();
}

RemoteData::Partial Aggregate::isSplittable() const {
    // Iterate over all the transactions, if any of them has
    // started and it can be splitted then continue else
    // just remove the block
    for (auto it = m_chunk.begin(); it != m_chunk.end(); it++) {
        if ((*it)->txn()->p_remoteData()->canPartial() == RemoteData::Partial::no)
            return RemoteData::Partial::no;
        else if ((*it)->txn()->p_remoteData()->canPartial() == RemoteData::Partial::yes)
            return RemoteData::Partial::yes;
    }
    return RemoteData::Partial::unknown;
}

bool Aggregate::isSplitReady() const {
    for (auto it = m_chunk.begin(); it != m_chunk.end(); ++it){
        // TODO Some hifi algorithm
        if ((*it)->txn()->speed()<=100 && !(*it)->txn()->isComplete())
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

    while (it < m_chunk.size()) {
        uintmax_t ibr = m_chunk[it]->txn()->bytesRemaining();
        if(ibr < m_splittable_size)
            continue;
        // If it is splittable, it is the bottleneck
        bneck = it;
        bbr = m_chunk[bneck]->txn()->bytesRemaining();
        btr = m_chunk[bneck]->txn()->timeRemaining();
        break;
        ++it;
    }

    // If there is no bottleneck Chunk then throw exception
    if(it == m_chunk.size())
        Throw(ex::aggregate::NoBottleneck);

    // Now just get the real bottle neck
    while (it < m_chunk.size()) {
        uintmax_t ibr = m_chunk[it]->txn()->bytesRemaining();
        if( ibr <= m_splittable_size)
            continue;
        uintmax_t itr = m_chunk[it]->txn()->timeRemaining();
        if((btr < itr) || (btr==itr && bbr<ibr)){
            bneck = it;
            bbr = m_chunk[bneck]->txn()->bytesRemaining();
            btr = m_chunk[bneck]->txn()->timeRemaining();
        }
        ++it;
    }

    return bneck;
}

// Split a Chunk and insert new Chunk after it
void Aggregate::split(std::vector<Chunk*>::size_type split_index){
    Chunk* cell = m_chunk[split_index];

    cell->txn()->pause();

    // Calculate the split point ie midpoint
    // Note: Range is exclusive of previous session
    uintmax_t downloaded = cell->txn()->bytesDone();
    uintmax_t lower = cell->txn()->range().lb();
    uintmax_t upper = cell->txn()->range().ub();
    uintmax_t midpoint = (upper+(lower+downloaded))/2;

    if( midpoint > upper || midpoint < lower)
        Throw(ex::Error,"Midpoint lies outside of lower and upper bounds.");

    // Create a new cloned BasicTransaction instance and update values
    // Create a new File and Chunk objects

    cell->txn()->updateRange(midpoint);

    File* newfile = new File(chunkName(midpoint));
    Range newrange(upper,midpoint);
    BasicTransaction* newtxn = cell->txn()->clone(newrange);
    Chunk* newcell = new Chunk(newtxn,newfile);

    // Insert newly created Chunk in the vector after
    m_chunk.insert(m_chunk.begin()+split_index+1, newcell);

    // Start those BasicTransactions
    cell->txn()->play();
    newcell->txn()->start();
}

void Aggregate::worker() try {
    //fancyprint("STARTER",NOTIFY);
    starter();
    //fancyprint("SPLITTER",NOTIFY);
    splitter();
    //fancyprint("JOIN ALL",NOTIFY);
    joinChunks();
    //fancyprint("MERGER",NOTIFY);
    merger();
} catch ( ex::Error e ) {
    fancyprint(e.what(),ERROR);
}

void Aggregate::starter() {
    // Directory session is used to find out about
    // previous downloads
    Directory session(m_hasedUrl);
    if ( session.exists() && !session.isEmpty() ) {

        std::vector<std::string> files = session.list(Node::FILE,true);
        // Remove non-numeric names
        for(unsigned i=0;i<files.size();i++){
            if(!isNumeric(files[i]))
                files.erase(files.begin()+i);
        }
        sort(files.begin(),files.end(),numerically);

        // Check the last entry for filesize
        // Last element name holds the total size of the download file
        m_filesize = std::atoi(files.back().c_str());
        if( File(chunkName(m_filesize)).size() != 0 )
            Throw(ex::Error,"Limiter file must have zero size.");

        // Start other chunks
        for(unsigned i=0; i < files.size()-1; i++){
            File* f = new File(chunkName(std::atoi(files[i].c_str())));
            Range r = Range(std::atoi(files[i+1].c_str()),std::atoi(files[i].c_str())+f->size());
            BasicTransaction* t= BasicTransaction::factory(m_url,r);
            Chunk* c = new Chunk(t,f);
            m_chunk.push_back(c);
            c->txn()->start();
        }

    } else {
        // Researcher finds about the necessary information
        // about the download file; filesize, resumability
        // NOTE: No range is sent to the BasicTransaction
        File* newfile = new File(chunkName(0));
        BasicTransaction* newtxn = BasicTransaction::factory(m_url);
        Chunk* researcher = new Chunk(newtxn,newfile);
        m_chunk.push_back(researcher);
        researcher->txn()->start();

        // Wait for researcher until downloading starts,
        // Now we get the proper information about the file
        // and further process can be started
        while (researcher->txn()->state() < BasicTransaction::State::downloading)
            boost::this_thread::sleep(boost::posix_time::millisec(100));

        if(researcher->txn()->state() == BasicTransaction::State::failed)
            Throw(ex::Error,"Starting transaction failed.");

        // Initialize m_filesize
        m_filesize = researcher->txn()->bytesTotal();

        // Create a limiter file
        File limiter(chunkName(m_filesize));
        limiter.write();
    }
}

void Aggregate::splitter() {

    // Check if any of the Chunk is splittable
    while (true){
        // Sleep
        boost::this_thread::sleep(boost::posix_time::millisec(100));

        RemoteData::Partial p = isSplittable();
        if (p==RemoteData::Partial::yes)
            break;
        else if (p==RemoteData::Partial::no)
            return;
    }

    // Loop while a bottleneck exists
    while (true){
        // Sleep
        boost::this_thread::sleep(boost::posix_time::millisec(100));

        // Get the bottle neck and split
        if(activeChunks() < m_chunks) {
            std::vector<Chunk*>::size_type bneck;
            try {
                bneck = bottleNeck();
            } catch (ex::aggregate::NoBottleneck) {
                break;
            }
            // NOTE: Removing this showed the synronization bug
            //if(isSplitReady())
            split(bneck);
        }
    }
}

void Aggregate::merger() {
    // If total size downloaded isn't equal
    // to the size of file downloaded then
    // do not merge the Chunks
    if( m_filesize != bytesTotal())
        Throw(ex::Error,"Downloaded bytes is greater than total filesize.");

    // New file to concatenate the Chunks
    File merged(prettyName());
    // Write an empty merge file first
    merged.write(Node::NEW);

    // Append the content to "merged" and remove
    // the chunk files
    for(auto it = m_chunk.begin(); it != m_chunk.end(); ++it){
        merged.append(*(*it)->file());
        (*it)->file()->remove();
    }
    // Remove the old directory
    Directory(m_hasedUrl).remove(Node::FORCE);
}