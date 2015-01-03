#include"aggregator/Aggregate.h"

Aggregate::Aggregate(const std::string& url, const std::string& destination,
        const std::string& purgatory, unsigned txns,uintmax_t split):
    m_chunks(txns), m_url(url), m_filesize(0), m_failed(false), m_thread(NULL),
    m_splittable_size( (split>100*1024) ? split : 100*1024), m_speed_thread(NULL),
    m_hashedUrl(purgatory+"/"+md5(removeport80(m_url))), m_prettyUrl(destination+"/"+prettify(m_url)),
    m_avgSpeed(0), m_instSpeed(0), m_hifiSpeed(0)
{
    // Directory session is used to find out about
    // previous download information
    std::vector<std::string> files;
    Directory session(m_hashedUrl);
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

    // Note: Files with numeric names
    if( files.size()==0) {

        // If no numeric files are found!
        // It means there is no previous session
        File* newfile = new File(chunkName(0));
        BasicTransaction* newtxn = BasicTransaction::factory(m_url);
        Chunk* researcher = new Chunk(newtxn,newfile);
        m_chunk.push_back(researcher);

    } else {

        // Get the file size from the last file
        m_filesize = std::atoi(files.back().c_str());

        // notifies that there is no limiter
        // deducded if last file isn't of zero size
        // or when there is only one file and which is 0;
        // exception thrown if last file is deleted
        // or if transaction isn't resumable;
        // NONRECOVERABLE
        // TODO delete everything and crash things
        if( File(chunkName(m_filesize)).size() != 0 || m_filesize==0)
            Throw(ex::filesystem::NotThere,"limiter");

        // occurs if first file is deleted
        // RECOVERABLE
        // Start file must always be 0
        // Insert a start file "0"
        if(files[0]!="0")
            files.insert(files.begin(),"0");

        // Populate all the Chunks
        for(unsigned i=0; i < files.size()-1; i++){
            uintmax_t start =std::atoi(files[i].c_str());
            uintmax_t end = std::atoi(files[i+1].c_str());
            File* f = new File(chunkName(std::atoi(files[i].c_str())));
            start += f->size();
            // These kind of files may be created due to
            // interruption in merging or faulty download
            if(start > end)
                start = end;
            Range r(end,start);
            BasicTransaction* t= BasicTransaction::factory(m_url,r);
            Chunk* c = new Chunk(t,f);
            m_chunk.push_back(c);
        }
    }
}

Aggregate::~Aggregate(){
    // Delete all Chunk and Socket
    stop();

    for (auto it = m_chunk.begin(); it != m_chunk.end(); ++it)
        delete (*it);
    if(m_thread)
        delete m_thread;
    if(m_speed_thread)
        delete m_speed_thread;
}

void Aggregate::stop(){
    for(auto it = m_chunk.begin(); it != m_chunk.end(); ++it)
        (*it)->txn()->stop();
    if(m_thread && m_thread->joinable())
        m_thread->interrupt();
    if(m_speed_thread && m_speed_thread->joinable())
        m_speed_thread->interrupt();
}

unsigned Aggregate::displayChunks() const {
    fancyprint(activeChunks() << "/" << totalChunks(),NOTIFY);

    int j = m_chunk.size();
    for(auto i=0;i<m_chunk.size();i++){
        uintmax_t lower = std::atoi(m_chunk[i]->file()->filename().c_str());
        uintmax_t down= m_chunk[i]->txn()->range().lb()+m_chunk[i]->txn()->bytesDone();
        uintmax_t higher = m_chunk[i]->txn()->range().ub();
        std::string myColor;

        if(m_chunk[i]->txn()->isComplete())
            myColor = SUCCESS;
        else if(m_chunk[i]->txn()->hasFailed())
            myColor = ERROR;
        else if(m_chunk[i]->txn()->isDownloading())
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
        if(!(*it)->txn()->isComplete())
            return false;
    }
    return true;
}

bool Aggregate::isFinished() const {
    for (auto it = m_chunk.begin(); it != m_chunk.end(); ++it){
        if(!(*it)->txn()->isComplete() && !(*it)->txn()->hasFailed())
            return false;
    }
    return true;
}

/*
   double Aggregate::speed() {
   double s = 0;
   for(auto it = m_chunk.begin();it != m_chunk.end();++it)
   s += (*it)->txn()->speed();
   return s;
   }
   */

unsigned Aggregate::activeChunks() const {
    unsigned count = 0;
    for (auto it = m_chunk.begin(); it != m_chunk.end(); ++it){
        if( (*it)->txn()->isRunning())
            count++;
    }
    return count;
}

void Aggregate::joinChunks() {
    for(auto it = m_chunk.begin(); it != m_chunk.end(); ++it)
        (*it)->txn()->join();
}

bool Aggregate::isSplitReady() const {
    // It is split ready if inactive count is less than 4
    unsigned count=0;
    for (auto it = m_chunk.begin(); it != m_chunk.end(); ++it){
        if(!(*it)->txn()->isDownloading())
            count++;
        if(count > 3)
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

    // Update the upper byte of the byte range.
    // no bytes beyond a certain point.
    cell->txn()->range().ub(midpoint);
    cell->txn()->play();

    Range newrange(upper,midpoint);
    File* newfile = new File(chunkName(midpoint));
    BasicTransaction* newtxn = cell->txn()->clone(newrange);
    Chunk* newcell = new Chunk(newtxn,newfile);

    // Insert newly created Chunk in the vector after
    m_chunk.insert(m_chunk.begin()+split_index+1, newcell);

    newcell->txn()->start();
}

void Aggregate::worker() try {
    //fancyprint("STARTER",NOTIFY);
    try {
        starter();
        //fancyprint("SPLITTER",NOTIFY);
        splitter();
    } catch (ex::aggregator::NonResumable& e){
        // The download wansn't resumable
        // so splitter() could be skipped
    }
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

    // If m_filesize is zero, then it is the first time
    // starting the transaction
    if(m_filesize==0) {
        // Researcher finds about the necessary information
        // about the download file; filesize, resumability
        Chunk* researcher = m_chunk[0];
        researcher->txn()->start();

        // Wait for researcher until downloading starts,
        // Now we get the proper information about the file
        while( !researcher->txn()->isDownloading() &&
                !researcher->txn()->isComplete() &&
                !researcher->txn()->hasFailed() )
            boost::this_thread::sleep(boost::posix_time::millisec(100));

        if(researcher->txn()->hasFailed())
            Throw(ex::Error,"Starting transaction failed.");

        // Initialize m_filesize
        m_filesize = researcher->txn()->bytesTotal();
        // TODO if no content-length then bytesTotal() must give zero and act accordingly inside
        // If no content-length is given then bytesTotal should
        // provide with zero size so it is non-resumable
        if(m_filesize!=0 && researcher->txn()->remoteData().canPartial() == RemoteData::Partial::yes){
            // Create a limiter file
            // this indicates that file can be resumed
            // as non-resumable downloads don't need
            // a total size file
            File limiter(chunkName(m_filesize));
            limiter.write();
        } else {
            Throw(ex::aggregator::NonResumable);
        }

    } else {
        // Start all the Chunks
        // They should be started at last
        for(auto it = m_chunk.begin();it != m_chunk.end(); it++)
            (*it)->txn()->start();
    }
}

void Aggregate::splitter() try {

    // Loop while a bottleneck exists
    while (!isFinished() && !hasFailed()) {
        // Get the bottle neck and split
        if(activeChunks() < m_chunks && isSplitReady() ) {
            // NOTE: Removing this showed the synronization bug
            std::vector<Chunk*>::size_type bneck = bottleNeck();
            split(bneck);
            // Just sleep for a while
            boost::this_thread::sleep(boost::posix_time::millisec(1000));
        }
        boost::this_thread::sleep(boost::posix_time::millisec(100));
    }

} catch (ex::aggregator::NoBottleneck& e) {
    // This isn't a bad thing, just signifies
    // that no further segmentation can occur.
    // It helps to get outside both of splitting
    // loops.
}

void Aggregate::merger() {

    // Just sleep for a while
    // Only for syncronization in main loop
    boost::this_thread::sleep(boost::posix_time::millisec(500));


    std::vector<File*> files;
    for(unsigned i=0; i < m_chunk.size();i++)
        files.push_back(m_chunk[i]->file());
    // first represents the "0" file and which is sure to
    // exist
    File* first = files[0];

    File limiter(chunkName(m_filesize));
    // If limiter doesn't exist then it implies that
    // download shouldn't be resumed and that imples
    // that nothing is splitted and hence no merging
    if( limiter.exists() ){
        fancyprint("Merging!",NOTIFY);

        // limiter is also added for calculations
        // but not processed
        files.push_back(&limiter);
        // first and last file aren't processed
        for(unsigned i=1; i < files.size()-1;i++){
            print(i << " of " << files.size()-1);

            uintmax_t prev_size = first->size();
            uintmax_t current_size = files[i]->size();
            uintmax_t current_expected_size = std::atoi(files[i]->filename().c_str());
            uintmax_t next_expected_size = std::atoi( files[i+1]->filename().c_str());

            // prev_size must lies in range [current_expected_size,next_expected_size]
            if( prev_size < current_expected_size)
                Throw(ex::Error,"Data is missing.");
            else if( prev_size > next_expected_size)
                Throw(ex::Error,"Data is redundant. We don't truncate.");

            uintmax_t offset = prev_size - current_expected_size;
            uintmax_t totalBytes = next_expected_size - current_expected_size;
            if (current_size < totalBytes)
                Throw(ex::Error,"Data is missing. We don't truncate.");
            // TODO improve append()
            // Here total=0 means that offset is equal to total
            // which doesn't imply writing everything
            totalBytes -= offset;
            if(totalBytes!=0)
                first->append(*files[i],totalBytes,offset);
            files[i]->remove();

            std::cout << DELETE;
        }
    }
    // Move the merged file to safe place
    first->move(prettyName(),Node::NEW);
    // Remove the old directory
    Directory(m_hashedUrl).remove(Node::FORCE);

    fancyprint("Complete!",SUCCESS);
}

double Aggregate::aggregateSpeed() const {
    double s = 0;
    for(auto it = m_chunk.begin();it != m_chunk.end();++it)
        s += (*it)->txn()->speed();
    return s;
}

void Aggregate::speed_worker() {
    const double refresh = 0.1;
    const unsigned persistance = 1/refresh*5;
    uintmax_t no = 0;

    while (!isFinished() && !hasFailed()){
        boost::this_thread::sleep(boost::posix_time::millisec(refresh*1000));
        m_instSpeed = aggregateSpeed();
        m_avgSpeed= (m_avgSpeed*no+m_instSpeed)/(no+1);
        int per = (no < persistance)?no:persistance;
        m_hifiSpeed= (m_hifiSpeed*per+m_instSpeed)/(per+1);
        no++;
    }

    m_instSpeed = 0;
    m_avgSpeed = 0;
    m_hifiSpeed = 0;
}
