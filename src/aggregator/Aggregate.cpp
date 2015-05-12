#include"aggregator/Aggregate.h"

Aggregate::Aggregate(const std::string& url, const std::string& destination,
                     const std::string& purgatory, unsigned txns, uintmax_t split):
    m_failed(false),
    m_chunks(txns), m_inactive_chunks(4),
    mptr_thread(NULL), mptr_speed_thread(NULL),
    m_splittable_size( max(split, 100 * 1024) ),
    m_url(url),
    m_purgatory_filename(purgatory + "/" + md5(removeport80(url))),
    m_destination_filename(destination + "/" + prettify(url)),
    m_filesize(0),
    m_avgSpeed(0), m_instSpeed(0), m_hifiSpeed(0) {

    m_txnExBridge = new ExBridge;

    // Directory session is used to find out about
    // previous download information
    std::vector<std::string> files;
    {
        Directory session(purgatoryFilename());
        if ( session.exists() && !session.isEmpty() ) {
            files = session.list(Node::FILE, true);
            // Remove non-numeric names
            for (unsigned i = 0; i < files.size(); i++) {
                if (!isNumeric(files[i]))
                    files.erase(files.begin() + i);
            }
            // sort the files numerically
            sort(files.begin(), files.end(), numerically);
        }
    }

    // Note: Files with numeric names
    if ( files.size() <= 0) {

        // If no numeric files are found, there is no previous session
        File* newfile = new File(chunkFilename(0));
        BasicTransaction* newtxn = BasicTransaction::factory(m_url);
        newtxn->exbridge(m_txnExBridge);
        Chunk* researcher = new Chunk(newtxn, newfile);
        m_chunk.push_back(researcher);

    } else {

        // Get the filesize from the filename of last file
        m_filesize = std::atoi(files.back().c_str());

        // Check for valid limiter, limiter should have zero size
        // and it must not be the first file
        // NONRECOVERABLE : occurs if last file is deleted
        if ( File(chunkFilename(m_filesize)).size() != 0 || m_filesize == 0)
            Throw(ex::filesystem::NotThere, "Limiter");

        // Check if start file is "0" else insert one
        // RECOVERABLE : occurs if first file is deleted
        if (files[0] != "0")
            files.insert(files.begin(), "0");

        // Populate all the Chunks
        for (unsigned i = 0; i < files.size() - 1; i++) {
            uintmax_t start = std::atoi(files[i].c_str());
            uintmax_t end = std::atoi(files[i + 1].c_str());
            File* f = new File(chunkFilename(start));
            start += f->size();

            // RECOVERABLE: occurs if interruption in merging
            Range r(end, min(start, end));
            BasicTransaction* t = BasicTransaction::factory(m_url, r);
            t->exbridge(m_txnExBridge);
            Chunk* c = new Chunk(t, f);
            m_chunk.push_back(c);
        }
    }
}

Aggregate::~Aggregate() {
    // Delete all Chunks
    stop();

    for (auto it = m_chunk.begin(); it != m_chunk.end(); ++it)
        delete (*it);
    if (mptr_thread)
        delete mptr_thread;
    if (mptr_speed_thread)
        delete mptr_speed_thread;
}

void Aggregate::stop() {
    for (auto it = m_chunk.begin(); it != m_chunk.end(); ++it)
        (*it)->txn()->stop();
    if (mptr_thread && mptr_thread->joinable())
        mptr_thread->interrupt();
    if (mptr_speed_thread && mptr_speed_thread->joinable())
        mptr_speed_thread->interrupt();
}

void Aggregate::displayExceptions() {
    int size = m_txnExBridge->number();
    for(int i=0;i<size;i++){
        fancyshow( m_txnExBridge->pop().what(),ERROR);
    }
}

unsigned Aggregate::displayChunks() const {

    // Progress bar
    std::cout << progressbar(progress(), BARONE, BARTWO)
        << formatTime(timeRemaining()) << "\t"
        << formatByte(speed()) << "ps\t"<< std::endl;

    int size = m_chunk.size();

    // Width of the maximum number
    int width = std::log10(m_chunk[size-1]->txn()->range().ub())+1;

    // Segments
    for (auto i = 0; i < size; i++) {
        uintmax_t lower = std::atoi(m_chunk[i]->file()->filename().c_str());
        uintmax_t down = m_chunk[i]->txn()->range().lb() + m_chunk[i]->txn()->bytesDone();
        uintmax_t higher = m_chunk[i]->txn()->range().ub();
        std::string myColor;

        if (m_chunk[i]->txn()->isComplete()){
            myColor = SUCCESS;
            // When there is success but downloaded size is invalid
            if(down != higher)
                myColor = COLOR(0,CC::PURPLE,CC::BLACK);
        } else if (m_chunk[i]->txn()->hasFailed())
            myColor = ERROR;
        else if (m_chunk[i]->txn()->isDownloading())
            myColor = WARNING;
        else
            myColor = NOTIFY;

        fancyshow(std::setfill('0')
                << std::setw(width)<< lower << ":"
                << std::setw(width) << down << ":"
                << std::setw(width) << higher, myColor);
    }

    // Two more lines for Status and progressbar.
    return size + 1;
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
    for (auto it = m_chunk.begin(); it != m_chunk.end(); ++it) {
        if (!(*it)->txn()->isComplete())
            return false;
    }
    return true;
}

bool Aggregate::isFinished() const {
    for (auto it = m_chunk.begin(); it != m_chunk.end(); ++it) {
        if (!(*it)->txn()->isComplete() && !(*it)->txn()->hasFailed())
            return false;
    }
    return true;
}

unsigned Aggregate::activeChunks() const {
    unsigned count = 0;
    for (auto it = m_chunk.begin(); it != m_chunk.end(); ++it) {
        if ( (*it)->txn()->isRunning())
            count++;
    }
    return count;
}

void Aggregate::joinChunks() {
    for (auto it = m_chunk.begin(); it != m_chunk.end(); ++it)
        (*it)->txn()->join();
}

bool Aggregate::isSplitReady() const {

    // It is split ready if inactive count is less than 4
    unsigned count = 0;
    for (auto it = m_chunk.begin(); it != m_chunk.end(); ++it) {
        if (!(*it)->txn()->isDownloading() &&
            !(*it)->txn()->isComplete() &&
            !(*it)->txn()->hasFailed())
            count++;
        if (count >= m_inactive_chunks)
            return false;
    }
    return true;
}

std::vector<Chunk*>::size_type Aggregate::bottleNeck() const {
    // Initialize bottleneck such that it is the first chunk
    // which is splittable

    // Bottleneck index
    std::vector<Chunk*>::size_type bneck = 0;
    // Iterator
    std::vector<Chunk*>::size_type it = 0;

    // Bottleneck Bytes Remaining
    uintmax_t bbr = 0;
    // Bottleneck Time Remaining
    uintmax_t btr = 0;

    // Get the first candidate for bottleneck
    for (; it < m_chunk.size(); ++it) {
        uintmax_t ibr = m_chunk[it]->txn()->bytesRemaining();
        if (ibr <= m_splittable_size)
            continue;
        // If it is splittable, it is the bottleneck
        bneck = it;
        bbr = m_chunk[bneck]->txn()->bytesRemaining();
        btr = m_chunk[bneck]->txn()->timeRemaining();
        break;
    }

    // If there is no bottleneck candidate then throw exception
    if (it == m_chunk.size())
        Throw(ex::aggregator::NoBottleneck);

    // Now get the real bottle neck
    for (; it < m_chunk.size(); ++it) {
        uintmax_t ibr = m_chunk[it]->txn()->bytesRemaining();
        if ( ibr <= m_splittable_size)
            continue;
        uintmax_t itr = m_chunk[it]->txn()->timeRemaining();
        if ((btr < itr) || (btr == itr && bbr < ibr)) {
            bneck = it;
            bbr = m_chunk[bneck]->txn()->bytesRemaining();
            btr = m_chunk[bneck]->txn()->timeRemaining();
        }
    }

    return bneck;
}

// Split a Chunk and insert new Chunk after it
void Aggregate::split(std::vector<Chunk*>::size_type split_index) {

    Chunk* cell = m_chunk[split_index];
    cell->txn()->pause();

    // Calculate the split point ie midpoint
    // Note: Range is exclusive of previous session
    uintmax_t downloaded = cell->txn()->bytesDone();
    uintmax_t lower = cell->txn()->range().lb();
    uintmax_t upper = cell->txn()->range().ub();
    uintmax_t midpoint = (upper + (lower + downloaded)) / 2;

    if ( midpoint > upper || midpoint < lower)
        Throw(ex::Invalid, "Midpoint");

    // Update the upper byte of the byte range.
    // no bytes beyond a certain point.
    cell->txn()->range().ub(midpoint);
    cell->txn()->play();

    Range newrange(upper, midpoint);
    File* newfile = new File(chunkFilename(midpoint));
    BasicTransaction* newtxn = cell->txn()->clone(newrange);
    newtxn->exbridge(m_txnExBridge);
    Chunk* newcell = new Chunk(newtxn, newfile);

    // Insert newly created Chunk in the vector after
    m_chunk.insert(m_chunk.begin() + split_index + 1, newcell);

    newcell->txn()->start();
}

void Aggregate::worker() try {
    try {
        //fancyshow("STARTER",NOTIFY);
        starter();
        //fancyshow("SPLITTER",NOTIFY);
        splitter();
    } catch (ex::aggregator::NonResumable& e) {
        // The download wasn't resumable
        // so splitter() could be skipped
    }
    //fancyshow("JOIN ALL",NOTIFY);
    joinChunks();

    // The errors in RemoteTransaction aren't thrown
    // Just logged in m_txnExBridge so a check must be done
    checkValidity();

    //fancyshow("MERGER",NOTIFY);
    merger();

} catch ( std::runtime_error e ) {
    m_failed = true;
    m_txnExBridge->log(e);
}

void Aggregate::starter() {

    // If m_filesize is zero, then it is the first time
    // starting the transaction
    if (m_filesize == 0) {
        // Researcher finds about the necessary information
        // about the download file; filesize, resumability
        Chunk* researcher = m_chunk[0];
        researcher->txn()->start();

        // Wait for researcher until downloading starts,
        // Now we get the proper information about the file
        while ( !researcher->txn()->isDownloading() &&
                !researcher->txn()->isComplete() &&
                !researcher->txn()->hasFailed() )
            boost::this_thread::sleep(boost::posix_time::millisec(100));

        if (researcher->txn()->hasFailed())
            Throw(ex::Failed, "Transaction start");

        // Initialize m_filesize
        m_filesize = researcher->txn()->bytesTotal();

        // If no filename is given then the default url-generated filename is used
        if (researcher->txn()->remoteData().filename() != "") {
            // Remove the filename-from-url and append filename-from-header
            int lastslash = m_destination_filename.find_last_of('/');
            m_destination_filename = m_destination_filename.substr(0, lastslash + 1) +
                                     researcher->txn()->remoteData().filename();
        }

        // If no content-length is given then bytesTotal should
        // provide with zero size so it is non-resumable
        if (m_filesize != 0 && researcher->txn()->remoteData().canPartial() == RemoteData::Partial::yes) {
            // Create a limiter file, indicates resumable file
            File limiter(chunkFilename(m_filesize));
            limiter.write();
        } else {
            Throw(ex::aggregator::NonResumable);
        }

        LocalOptions l;
        l.store(purgatoryFilename() + "/" + localConfig);
        l.load();
        // l.variablesMap().at("transaction.filename").value() = m_destination_filename;

        // NOTE: Resumability and Filesize saved using file arrangement
        // so these values aren't saved in local ini.
        l.variablesMap().insert(std::make_pair("transaction.filename",
            po::variable_value(m_destination_filename, true)));

        l.unload(purgatoryFilename() + "/" + localConfig);

    } else {
        // Start all the Chunks
        // They should be started at last
        for (auto it = m_chunk.begin(); it != m_chunk.end(); it++)
            (*it)->txn()->start();
    }
}

void Aggregate::splitter() try {

    // Loop while a bottleneck exists
    while (!isFinished() && !hasFailed()) {
        // Get the bottle neck and split
        if (activeChunks() < m_chunks && isSplitReady() ) {
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
    for (unsigned i = 0; i < m_chunk.size(); i++)
        files.push_back(m_chunk[i]->file());
    // first represents the "0" file and which is sure to exist
    File* first = files[0];

    File limiter(chunkFilename(m_filesize));
    // If limiter doesn't exist then it implies that
    // download shouldn't be resumed and that imples
    // that nothing is splitted and hence no merging
    if ( limiter.exists() ) {
        fancyshow("Merging!", NOTIFY);

        // limiter is also added for calculations
        // but not processed
        files.push_back(&limiter);
        // first and last file aren't processed
        uintmax_t prev_size = first->size();
        for (unsigned i = 1; i < files.size() - 1; i++) {

            // Status bar
            show(i << " of " << files.size() - 1);
            // Progress bar
            std::cout << progressbar(100.0*i/(files.size()-1), BARONE, BARTWO)<< std::endl;

            uintmax_t current_size = files[i]->size();
            uintmax_t current_expected_size = std::atoi(files[i]->filename().c_str());
            uintmax_t next_expected_size = std::atoi( files[i + 1]->filename().c_str());

            // prev_size must lies in range [current_expected_size,next_expected_size]
            if ( prev_size < current_expected_size)
                Throw(ex::Error, "Data is missing.");
            else if ( prev_size > next_expected_size)
                Throw(ex::Error, "Data is redundant. We don't truncate.");

            uintmax_t offset = prev_size - current_expected_size;
            uintmax_t totalBytes = next_expected_size - current_expected_size;
            if (current_size < totalBytes)
                Throw(ex::Error, "Data is missing.");
            totalBytes -= offset;
            first->append(*files[i], offset, totalBytes);
            prev_size += current_size;
            files[i]->remove();

            for (int i = 0; i < 2; i++)
                std::cout << DELETE;
        }
        // Delete "Merging!"
        std::cout << DELETE;
    }
    // Move the merged file to safe place
    first->move(destinationFilename(), Node::NEW);
    // Remove the old directory
    Directory(purgatoryFilename()).remove(Node::FORCE);

    fancyshow("Complete!", SUCCESS);
}

double Aggregate::aggregateSpeed() const {
    double s = 0;
    for (auto it = m_chunk.begin(); it != m_chunk.end(); ++it)
        s += (*it)->txn()->speed();
    return s;
}

void Aggregate::checkValidity() const {
    for (auto it = m_chunk.begin(); it != m_chunk.end(); ++it)
        if((*it)->txn()->hasFailed())
            Throw(ex::Invalid,"Transaction");
}



void Aggregate::speed_worker() {
    const double refresh = 0.1;
    const unsigned persistance = 1 / refresh * 5;
    uintmax_t no = 0;

    while (!isFinished() && !hasFailed()) {
        boost::this_thread::sleep(boost::posix_time::millisec(refresh * 1000));
        m_instSpeed = aggregateSpeed();
        m_avgSpeed = (m_avgSpeed * no + m_instSpeed) / (no + 1);
        int per = (no < persistance) ? no : persistance;
        m_hifiSpeed = (m_hifiSpeed * per + m_instSpeed) / (per + 1);
        no++;
    }

    // Speed must be reset to zero after download is complete.
    m_instSpeed = 0;
    m_avgSpeed = 0;
    m_hifiSpeed = 0;
}



