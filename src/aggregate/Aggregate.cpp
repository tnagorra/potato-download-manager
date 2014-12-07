#include"aggregate/Aggregate.h"

/*
   When BasicTransaction is complete, push those to m_socket
   When BasicTransaction is started and m_socket isn't empty,
   take it from m_socket and pop it
   if it is empty, create new inside the BasicTransaction
*/

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

void Aggregate::joinAll(){
    // Join all Chunks
    for(auto it = m_chunk.begin(); it != m_chunk.end(); ++it){
        (*it)->txn()->join();
    }
}

void Aggregate::join(){
    // TODO have check
    m_thread.join();
}

void Aggregate::stop(){
    /* TODO stop everything
       for(auto it = m_chunk.begin(); it != m_chunk.end(); ++it){
       it->txn()->stop();
       }
       */
}

void Aggregate::start() {
    // TODO have check
    m_thread = boost::thread(&Aggregate::worker,this);
}

uintmax_t Aggregate::bytesDone() const {
    uintmax_t bytes = 0;
    for (auto it = m_chunk.begin(); it != m_chunk.end(); ++it)
        bytes += (*it)->bytesDone();
    return bytes;
}

uintmax_t Aggregate::bytesTotal() const {
    return m_filesize;
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
        if ((*it)->txn()->isRunning())
            count++;
    }
    return count;
}
unsigned Aggregate::totalChunks() const {
    return m_chunk.size();
}

double Aggregate::speed() const {
    double s = 0;
    for(auto i = m_chunk.begin();i != m_chunk.end();++i)
        s += (*i)->txn()->speed();
    return s;
}

double Aggregate::progress() const {
    if( m_filesize == 0 ) return 0;
    return 1.0*bytesDone()/m_filesize*100;
}

uintmax_t Aggregate::timeRemaining() const {
    // TODO check if bytesTotal is less than bytesDone
    double spd = speed();
    if(spd == 0)
        return std::numeric_limits<uintmax_t>::max();
    return (bytesTotal()-bytesDone())/spd;
}

bool Aggregate::complete() const {
    for (auto it = m_chunk.begin(); it != m_chunk.end(); ++it){
        if( (*it)->txn()->isComplete() == false )
            return false;
    }
    return true;
}

bool Aggregate::splittable() const {
    for (auto it = m_chunk.begin(); it != m_chunk.end(); ++it){
        if((*it)->txn()->bytesRemaining() > m_splittable_size)
            return true;
    }
    return false;
}

bool Aggregate::splitReady() const {
    for (auto it = m_chunk.begin(); it != m_chunk.end(); ++it){
        // TODO 0 Insert some really good algorithm
        // Maybe wait for download to get to stable state
        // Waiting for a while may be benificial in long run
        if ((*it)->txn()->speed()<=10 && !(*it)->txn()->isComplete())
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
        Throw(ex::aggregate::NoBottleneck);
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

void Aggregate::display() {
    fancycout(activeChunks() << "/" << totalChunks() << " ",ERROR);
    for(auto i=0;i<m_chunk.size();i++){
        if( m_chunk[i]->txn()->isComplete() )
            fancycout(m_chunk[i]->file()->filename().c_str() << " ",SUCCESS);
        else if( m_chunk[i]->txn()->state() >= BasicTransaction::State::downloading)
            fancycout(m_chunk[i]->file()->filename().c_str() << " ",NOTIFY);
        else
            fancycout(m_chunk[i]->file()->filename().c_str() << " ",WARNING);
    }
    fancycout(m_filesize,SUCCESS);
    std::cout << std::endl;
}

// Split a Chunk and insert new Chunk after it
void Aggregate::split(std::vector<Chunk*>::size_type split_index){
    Chunk* cell = m_chunk[split_index];

    cell->txn()->pause();
    //fancyprint("Pause",NOTIFY);
    while(!cell->txn()->isPaused())
        boost::this_thread::sleep(boost::posix_time::millisec(100));
    //fancyprint("Pause complete",SUCCESS);

    // Calculate the split point ie midpoint
    // Note: Range is exclusive of previous session
    uintmax_t downloaded = cell->txn()->bytesDone();
    uintmax_t lower = cell->txn()->range().lb();
    uintmax_t upper = cell->txn()->range().ub();
    uintmax_t midpoint = (upper+(lower+downloaded))/2;

    if( midpoint > upper || midpoint < lower)
        Throw(ex::Invalid,"Range","Midpoint");
    // TODO can return void, as this means the split won't be
    // necessary

    //std::cout << "U " << upper << " L " << lower <<std::endl;
    //std::cout << "D " << downloaded << " M " << midpoint << std::endl;

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

void Aggregate::worker(){
    //fancyprint("STARTER",NOTIFY);
    starter();
    //fancyprint("SPLITTER",NOTIFY);
    splitter();
    //fancyprint("JOIN ALL",NOTIFY);
    joinAll();
    //fancyprint("MERGER",NOTIFY);
    merger();
}

void Aggregate::merger() {
    // TODO
    // If total size downloaded isn't equal
    // to the size of file downloaded then
    // do not merge the Chunks

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

void Aggregate::splitter() {
    // Loop while a bottleneck exists
    while (!complete()){
        // Sleep
        boost::this_thread::sleep(boost::posix_time::millisec(100));

        // Get the bottle neck and split
        // NOTE: There may be some problem here
        //print("here");
        if(activeChunks() < m_chunks) {
            std::vector<Chunk*>::size_type bneck;
            try {
                bneck = bottleNeck();
            } catch (ex::aggregate::NoBottleneck) {
                break;
            }
            //print("there");
            if(splitReady())
                split(bneck);
        }
    }
}

void Aggregate::starter() {
    // Directory session is used to find out about
    // previous downloads
    Directory session(m_hasedUrl);
    if ( session.exists() && !session.isEmpty() ) {

        //print("a");
        // TODO some case for files other than
        // numeric in nature
        // solution: use folders
        std::vector<std::string> files = session.list(Node::FILE,true);
        sort(files.begin(),files.end(),numerically);

        //print("b");
        // Last element name holds the total size of the download file
        m_filesize = std::atoi(files[files.size()-1].c_str());
        //print("c");

        // "starter" indicates the first Chunk usable
        // from the list of sorted Chunks
        // A complete Transaction isn't started again


        for(unsigned i=0; i < files.size()-1; i++){
            File* f = new File(chunkName(std::atoi(files[i].c_str())));
            Range r = Range(std::atoi(files[i+1].c_str()),std::atoi(files[i].c_str())+f->size());
            BasicTransaction* t= BasicTransaction::factory(m_url,r);
            Chunk* c = new Chunk(t,f);
            m_chunk.push_back(c);
            c->txn()->start();
        }

        /*
           unsigned istarter=0;
           Chunk* researcher = NULL;
           for(unsigned i=0;i<files.size()-1;i++){
           File* f = new File(chunkName(std::atoi(files[i].c_str())));
           uintmax_t s1 = f->size();
           uintmax_t s2 = std::atoi(files[i+1].c_str()) - std::atoi(files[i].c_str());
           if(s1 < s2) {
        // If not complete, it is the reseacher
        istarter = i;
        Range r = Range(std::atoi(files[i+1].c_str()),std::atoi(files[i].c_str())+f->size());
        BasicTransaction* t = BasicTransaction::factory(m_url,r);
        researcher = new Chunk(t,f);
        researcher->txn()->start();
        break;
        } else if (s1 == s2){
        delete f;
        // TODO We could push these files in a
        // queue, and not delete it because we
        // need it later
        } else {
        delete f;
        Throw(ex::Invalid,"Filesize",", exceeds acceptable value, ");
        }
        }

        //print("d");
        // No need to start the download
        // It is already complete
        if(researcher == NULL) {

        //print("d1");
        for(unsigned i=0; i < files.size()-1; i++){
        File* f = new File(chunkName(std::atoi(files[i].c_str())));
        Range r = Range(std::atoi(files[i+1].c_str()),std::atoi(files[i].c_str())+f->size());
        BasicTransaction* t= BasicTransaction::factory(m_url,r);
        Chunk* c = new Chunk(t,f);
        m_chunk.push_back(c);
        }
        //print("d2");
        Throw(ex::aggregate::AlreadyComplete);
        }
        //print("e");

        // Wait for researcher until downloading starts,
        // Now we get the proper information about the file
        // and further process can be started
        while (researcher->txn()->state() < BasicTransaction::State::downloading)
        boost::this_thread::sleep(boost::posix_time::millisec(100));

        //print("f");


        for(unsigned i=0; i < files.size()-1; i++){
        if(i==istarter){
        m_chunk.push_back(researcher);
        } else {
        File* f = new File(chunkName(std::atoi(files[i].c_str())));
        Range r = Range(std::atoi(files[i+1].c_str()),std::atoi(files[i].c_str())+f->size());
        BasicTransaction* t= researcher->txn()->clone(r);
        Chunk* c = new Chunk(t,f);
        m_chunk.push_back(c);
        c->txn()->start();
        }
        }
        //print("g");
        */

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

        // Initialize m_filesize
        m_filesize = researcher->txn()->bytesTotal();

        // Create a limiter file
        File limiter(chunkName(m_filesize));
        limiter.write();
    }
}
