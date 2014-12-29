#include"aggregator/Aggregate.h"

Aggregate::Aggregate(const std::string& url, const std::string& heaven,
        const std::string& purgatory, unsigned txns,uintmax_t split):
    m_chunks(txns), m_url(url), m_splittable_size(split),m_purgatory(purgatory),
    m_filesize(0), m_heaven(heaven), m_failed(false), m_thread(NULL)
{
    // Check to ensure m_splittable_size is not less than 100 KiB
    if(m_splittable_size < 100*1024)
        m_splittable_size = 100*1024;
    m_hashedUrl = m_purgatory+"/"+md5(m_url);
    m_prettyUrl = m_heaven+"/"+prettify(m_url);

    // Directory session is used to find out about
    // previous download information
    Directory session(m_hashedUrl);
    std::vector<std::string> files;

    if ( session.exists() && !session.isEmpty() ) {
        files = session.list(Node::FILE,true);
        // Remove non-numeric names
        for(unsigned i=0;i<files.size();i++){
            if(!isNumeric(files[i]))
                files.erase(files.begin()+i);
        }
        // sort the files numerically
        sort(files.begin(),files.end(),numerically);
    }

    // If no numeric files are found!
    if( files.size()==0) {

        File* newfile = new File(chunkName(0));
        BasicTransaction* newtxn = BasicTransaction::factory(m_url);
        Chunk* researcher = new Chunk(newtxn,newfile);
        m_chunk.push_back(researcher);

    } else {

        m_filesize = std::atoi(files.back().c_str());

        // ie, the last file must not be with name "0"
        if(m_filesize==0)
            Throw(ex::filesystem::ZeroSize);
        // ie, limiter file should be empty
        if( File(chunkName(m_filesize)).size() != 0 )
            Throw(ex::filesystem::NonZeroSize,"limiter");
        // ie, '0' could be missing
        if(files[0]!="0")
            files.insert(files.begin(),"0");

        // Populate all the Chunks
        for(unsigned i=0; i < files.size()-1; i++){
            File* f = new File(chunkName(std::atoi(files[i].c_str())));
            Range r(std::atoi(files[i+1].c_str()),std::atoi(files[i].c_str())+f->size());
            BasicTransaction* t= BasicTransaction::factory(m_url,r);
            Chunk* c = new Chunk(t,f);
            m_chunk.push_back(c);
        }
    }
}

Aggregate::~Aggregate(){
    // Delete all Chunk and Socket
    for (auto it = m_chunk.begin(); it != m_chunk.end(); ++it)
        delete (*it);
    if(m_thread)
        delete m_thread;
}

void Aggregate::stop(){
    for(auto it = m_chunk.begin(); it != m_chunk.end(); ++it)
        (*it)->txn()->stop();
    if(m_thread && m_thread->joinable())
        m_thread->interrupt();
}

unsigned Aggregate::displayChunks() {
    int j;
    fancyprint(activeChunks() << "/" << totalChunks(),NOTIFY);

    j = m_chunk.size();
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

    std::cout << progressbar(progress(),COLOR(0,CC::WHITE,CC::PURPLE),COLOR(0,CC::PURPLE,CC::WHITE));
    print( " " << round(progress(),2) << "%\t"
            << formatTime(timeRemaining()) << "\t"
            << formatByte(speed()) << "ps\t");

    return j+1;
}


uintmax_t Aggregate::bytesDone() {
    uintmax_t bytes = 0;
    for (auto it = m_chunk.begin(); it != m_chunk.end(); ++it)
        bytes += (*it)->bytesDone();
    return bytes;
}

uintmax_t Aggregate::bytesTotal(){
    uintmax_t bytes = 0;
    for (auto it = m_chunk.begin(); it != m_chunk.end(); ++it)
        bytes += (*it)->bytesTotal();
    return bytes;
}

bool Aggregate::isComplete() {
    for (auto it = m_chunk.begin(); it != m_chunk.end(); ++it){
        if( (*it)->txn()->isComplete() == false )
            return false;
    }
    return true;
}

double Aggregate::speed() {
    double s = 0;
    for(auto it = m_chunk.begin();it != m_chunk.end();++it)
        s += (*it)->txn()->speed();
    return s;
}

unsigned Aggregate::activeChunks() {
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

RemoteData::Partial Aggregate::isSplittable() {
    // Iterate over all the transactions, if any of them has
    // started and it can be splitted then return yes

    // Stores if all Chunks was complete
    bool pcomplete = true;
    for (auto it = m_chunk.begin(); it != m_chunk.end(); it++) {
        if(!(*it)->txn()->isComplete()){
            if ((*it)->txn()->remoteData().canPartial() == RemoteData::Partial::no)
                return RemoteData::Partial::no;
            else if ((*it)->txn()->remoteData().canPartial() == RemoteData::Partial::yes)
                return RemoteData::Partial::yes;
            pcomplete = false;
        }
    }
    if(pcomplete)
        Throw(ex::aggregator::NoBottleneck);
    return RemoteData::Partial::unknown;
}

bool Aggregate::isSplitReady() {

    for (auto it = m_chunk.begin(); it != m_chunk.end(); ++it){
        // TODO Some hifi algorithm
        if ((*it)->txn()->speed()<=10 && !(*it)->txn()->isComplete())
            return false;
    }
    return true;
}

std::vector<Chunk*>::size_type Aggregate::bottleNeck() {
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
    if( it == m_chunk.size() ) {
        Throw(ex::aggregator::NoBottleneck);
    }

    // Now just get the real bottle neck
    for (; it < m_chunk.size(); ++it){
        uintmax_t ibr = m_chunk[it]->txn()->bytesRemaining();
        if( ibr <= m_splittable_size)
            continue;
        uintmax_t itr = m_chunk[it]->txn()->timeRemaining();
        if((btr < itr) || (btr==itr && bbr<ibr)){
            bneck = it;
            bbr = m_chunk[bneck]->txn()->bytesRemaining();
            btr = m_chunk[bneck]->txn()->timeRemaining();
        }
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
        Throw(ex::Invalid,"Midpoint");


    Range newrange(upper,midpoint);
    File* newfile = new File(chunkName(midpoint));
    BasicTransaction* newtxn = cell->txn()->clone(newrange);
    Chunk* newcell = new Chunk(newtxn,newfile);
    newcell->txn()->start();

    // Insert newly created Chunk in the vector after
    m_chunk.insert(m_chunk.begin()+split_index+1, newcell);


    boost::this_thread::sleep(boost::posix_time::millisec(1000));
    // Update the upper byte of the byte range.
    // no bytes beyond a certain point.
    cell->txn()->range().ub(midpoint);
    cell->txn()->play();

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
    m_failed = true;
    //fancyprint(e.what(),ERROR);
    std::cout << e.what() << std::endl;
}

void Aggregate::starter() {

    // If there is only one chunk then it is the first
    // download chunk so infomation must be gathered

    if(m_filesize==0) {

        // Researcher finds about the necessary information
        // about the download file; filesize, resumability
        // NOTE: No range is sent to the BasicTransaction
        Chunk* researcher = m_chunk[0];
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

    } else {
        // Start all the Chunks
        // They should be started at last
        for(auto it = m_chunk.begin();it != m_chunk.end(); it++)
            (*it)->txn()->start();
    }

}

void Aggregate::splitter() try {

    // Check if any of the Chunk is splittable
    while (true){
        // Sleep
        boost::this_thread::sleep(boost::posix_time::millisec(100));

        RemoteData::Partial p = isSplittable();
        if (p == RemoteData::Partial::yes)
            break;
        else if (p == RemoteData::Partial::no)
            return;
    }

    // Loop while a bottleneck exists
    while (true){
        // Sleep
        boost::this_thread::sleep(boost::posix_time::millisec(100));

        // Get the bottle neck and split
        //print( activeChunks() << " " << m_chunks);

        if(activeChunks() < m_chunks) {
            std::vector<Chunk*>::size_type bneck = bottleNeck();

            // NOTE: Removing this showed the synronization bug
            //if(isSplitReady())
            split(bneck);
        }
    }
} catch (ex::aggregator::NoBottleneck e) {
    // This isn't a bad thing, just signifies
    // that no further segmentation can occur.
    // It helps to get outside both of splitting
    // loops.
}

void Aggregate::merger() {

    // Just sleep for a while
    // Only for syncronization in main loop
    boost::this_thread::sleep(boost::posix_time::millisec(500));

    // If total size downloaded isn't equal
    // to the size of file downloaded then
    // do not merge the Chunks

    if( m_filesize != bytesTotal())
        Throw(ex::Error,"Downloaded bytes greater than total filesize.");

    File tmp(tempName());
    tmp.write(Node::FORCE);

    // Append the content to "tmp"
    for(unsigned i=0; i < m_chunk.size(); i++){
        std::cout << "Merging! " <<  i+1  << " of " << m_chunk.size() <<std::endl;
        tmp.append(*(m_chunk[i]->file()));
        std::cout << DELETE;
    }

    tmp.move(prettyName(),Node::NEW);

    // Remove the old directory
    Directory(m_hashedUrl).remove(Node::FORCE);

    fancyprint("Complete!",SUCCESS);
}
