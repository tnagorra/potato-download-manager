// File File.cpp
// Contains the implementation for the File class.

#include "File.h"

// Asserts that the filename is clean for portability.
// That means the function does nothing if the filename is
// portable, and throws ex::Invalid if its not.
void File::assertClean() {
    /* TODO
       if( !fs::portable_file_name( m_name.filename().string() ))
       Throw(ex::Invalid,"File Name",m_name.filename().string());
       */
    Node::assertClean();
    if(exists() && what() != FILE)
        Throw(ex::Not,"File",m_name.string());
}

// Used to copy n bytes of data from input stream to m_stream
// If n is 0, then total input stream is copied
void File::streamCopy(std::istream& in, uintmax_t count) {
    if (count == 0){
        m_stream << in.rdbuf();
    } else {
        const uintmax_t bufferSize= 4096;
        char buffer[bufferSize];
        while(count > bufferSize){
            in.read(buffer, bufferSize);
            m_stream.write(buffer, bufferSize);
            count -= bufferSize;
        }
        in.read(buffer, count);
        m_stream.write(buffer, count);
    }
    // TODO maybe the flush isn't required
    // if m_stream is deleted correctly
    m_stream.flush();
}

// Constructor of the object.
// Throws ex::Not if given node not a file.
File::File(const fs::path& name, Mode mode):
    Node(name),m_mode(mode),m_state(CLOSED)
{
    assertClean();
}

// Destructor
File::~File() {
    close();
}

// Returns the size of the File
// Throws fs::filesystem_error
uintmax_t File::size() const {
    if (exists())
        return fs::file_size(m_name);
    return 0;
}

// Removes the File
// Returns False if file didn't exist in the first place,
// else returns True
// Throws fs::filesystem_error
int File::remove() {
    if(exists()){
        close();
        return fs::remove(m_name);
    }
    return 0;
}

// Copies a File to another file
// Throws fs::filesystem_error, ex::file::AlreadyThere,
// ex::file::NotThere
void File::copy(const fs::path& tos,Conflict c) {
    fs::path toss = tos;
    if (!exists())
        Throw(ex::file::NotThere,path().string());
    // To consider paths like "hari/shiva/"
    Node to(tos);
    if( to.exists() && to.what() == DIRECTORY || (to.filename() == "." || to.filename() == ".."))
        toss /= filename();
    File too(toss);
    if( too.exists() && c==LEAVE)
        Throw(ex::file::AlreadyThere,too.path().string());
    close();
    if( too.path().has_parent_path())
        Directory(too.parentpath().string()).create();
    fs::copy(path(),too.path());
}

// Moves a file
// Throws fs::filesystem_error, ex::file::AlreadyThere,
// ex::file::NotThere
void File::move(const fs::path& tos,Conflict c) {
    fs::path toss = tos;
    if (!exists())
        Throw(ex::file::NotThere,m_name.string());
    Node to(tos);
    if( to.exists() && to.what() == DIRECTORY || (to.filename() == "." || to.filename() == ".."))
        toss /= filename();
    File too(toss);
    if( too.exists() && c==LEAVE)
        Throw(ex::file::AlreadyThere,too.path().string());
    close();
    if( too.path().has_parent_path())
        Directory(too.parentpath().string()).create();
    fs::rename(path(),too.path());
    path(too.path());
}

// Closes the opened file
void File::close() {
    m_state = CLOSED;
    if(m_stream.is_open())
        m_stream.close();
}

// Closes the opened file
bool File::closeIfNot(State stat) {
    if (stat == CLOSED)
        Throw(ex::Invalid,"Mode",m_name.string());
    if (m_state == stat)
        return false;
    close();
    return true;
}

// Opens the file for reading, writing or appending
// Creates directories if necessary. Throws ex::Error if
// opening fails.
// NOTE: A file is opened only once if the mode is same,
// so reopening a file with same state does nothing.
void File::open(State mode) {
    if(!closeIfNot(mode))
        return;

    std::ios_base::openmode om;
    if (mode == READ)
        om = std::ios::in;
    else if (mode == WRITE)
        om = std::ios::out | std::ios::trunc;
    else if (mode == APPEND)
        om = std::ios::out | std::ios::app;

    if( m_mode == BINARY)
        om |= std::ios::binary;

    if( (mode == WRITE || mode == APPEND) && m_name.has_parent_path())
        Directory(m_name.parent_path().string()).create();

    m_stream.open(m_name.string().c_str(),om);
    if(m_stream.fail())
        Throw(ex::Error,"Opening \""+m_name.string()+"\" failed. Mode:" + std::to_string(mode));
    else
        m_state = mode;
}

// Creates an empty File
// Throws ex::file::AlreadyThere
void File::write(Conflict c) {
    if( c == LEAVE && exists())
        Throw(ex::file::AlreadyThere,m_name.string());
    open(WRITE);
}

// Creates a File using string, B+T
// Throws ex::file::AlreadyThere
void File::write(const std::string& data,Conflict c) {
    if( c == LEAVE && exists())
        Throw(ex::file::AlreadyThere,m_name.string());
    open(WRITE);
    m_stream << data;
}

// Creates a File using istream, B+T ??
// Throws ex::file::AlreadyThere
void File::write(std::istream& data,Conflict c,uintmax_t n){
    if( c == LEAVE && exists())
        Throw(ex::file::AlreadyThere,m_name.string());
    open(WRITE);
    streamCopy(data, n);
}

// Appends to a existing File using string, B+T
void File::append(const std::string& data) {
    open(APPEND);
    m_stream << data;
}

// Appends to a existing File using File, B+T
void File::append(File& data) {
    open(APPEND);
    data.open(READ);
    m_stream << data.m_stream.rdbuf();
}

// Appends to a existing File using istream, B+T
void File::append(std::istream& data,uintmax_t n){
    open(APPEND);
    streamCopy(data, n);
}

// Returns the line string from an existing File, T
// Throw ex::Error, ex::End, ex::file::NotThere
std::string File::readLine() {
    if (!exists())
        Throw(ex::file::NotThere,m_name.string());
    if(m_mode == BINARY)
        Throw(ex::Error,"Reading binary file line by line.");
    open(READ);
    std::string name;
    if( !std::getline(m_stream,name) )
        Throw(ex::file::End,m_name.string());
    return name;
}

// Returns a word string from an existing File, T
// Throw ex::Error, ex::End, ex::file::NotThere
std::string File::readWord() {
    if (!exists())
        Throw(ex::file::NotThere,m_name.string());
    if(m_mode == BINARY)
        Throw(ex::Error,"Reading binary file word by word.");
    open(READ);
    std::string name;
    m_stream >> name;
    if(name.length() == 0)
        Throw(ex::file::End,m_name.string());
    return name;
}

// End File File.cpp
