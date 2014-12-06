// File Directory.cpp
// Cointains implementation for class Directory

#include "filesystem/Directory.h"

// Asserts that the filename is clean for portability.
// That means the function does nothing if the filename is
// portable, and throws ex::Invalid if its not.
void Directory::assertClean(){
    /* TODO
       if( !fs::portable_name( m_name.filename().string() ))
       Throw(ex::Invalid,"Directory name",m_name.filename().string());
       */
    Node::assertClean();
    if(exists() && what() != DIRECTORY)
        Throw (ex::Not,"Directory",m_name.string());
}

// Constructor
// Throws ex::Not
Directory::Directory(const fs::path& name) : Node(name) {
    assertClean();
}

// Lists files and returns the vector of string
// Throws fs::filesystem_error, ex::filesystem::NotThere
std::vector<std::string> Directory::list(Type type, bool onlyfilename){
    if (!exists())
        Throw(ex::filesystem::NotThere,m_name.string());
    std::vector<std::string> myList;
    for ( fs::directory_iterator end, dir(m_name); dir != end; ++dir ) {
        Node node(dir->path());
        if( node.what() == type)
            if( onlyfilename ) {
                myList.push_back( node.path().filename().string());
            } else {
                myList.push_back( node.path().string());
            }
    }
    return myList;
}

// Lists files and returns the vector of string
// Throws fs::filesystem_error, ex::filesystem::NotThere
std::vector<std::string> Directory::listrecursive(Type type, bool onlyfilename){
    if (!exists())
        Throw(ex::filesystem::NotThere,m_name.string());
    std::vector<std::string> myList;
    for ( fs::recursive_directory_iterator end, dir(m_name); dir != end; ++dir ) {
        Node node(dir->path());
        if( node.what() == type)
            if( onlyfilename ) {
                myList.push_back( node.path().filename().string());
            } else {
                myList.push_back( node.path().string());
            }
    }
    return myList;
}

// Returns if the Directory is empty
// Throws fs::filesystem_error
bool Directory::isEmpty() const {
    return fs::is_empty(m_name);
}

// Create a Directory
// Throws fs::filesystem_error
void Directory::create(){
    if (!exists())
        fs::create_directories(m_name);
}

// Removes Directories recursively/nonrecursively
// Returns the number of files removed.
// Throws fs::filesystem_error
int Directory::remove(Conflict c){
    if (!exists())
        return 0;
    switch( c ){
        case FORCE: return fs::remove_all(m_name);
        case LEAVE: return fs::remove(m_name);
        default: return 0;
    }
}

// Copies Directories
// Throws fs::filesystem_error, ex::filesystem::AlreadyThere,
// ex::filesystem::NotThere, ex::Invalid
void Directory::copy(const fs::path& tos,Conflict c){
    Directory to(tos);
    if (!exists())
        Throw(ex::filesystem::NotThere,path().string());
    if ( to.exists() && !to.isEmpty() && c==LEAVE)
        Throw(ex::filesystem::AlreadyThere,to.path().string());
    // To consider paths like "hari/shiva/"
    if( to.filename() == "." || to.filename() == "..")
        to.path( to.path() / filename());

    std::vector<std::string> filelist = list(FILE);
    for( unsigned i = 0;i<filelist.size();i++){
        fs::path node = filelist[i];
        File f(node.string());
        f.copy(to.path()/node.filename(),c);
    }
    // Recursive copy
    std::vector<std::string> directorylist = list(DIRECTORY);
    for( unsigned i = 0;i<directorylist.size();i++){
        fs::path node = directorylist[i];
        Directory( to.path() / node.filename() ).create();
        Directory d( node.string() );
        d.copy(to.path()/node.filename(),c);
    }
}

// Moves a Directory
// Throws fs::filesystem_error, ex::filesystem::AlreadyThere,
// and ex::filesystem::NotThere
void Directory::move(const fs::path& tos, Conflict c){
    Directory to(tos);
    if (!exists())
        Throw(ex::filesystem::NotThere,path().string());
    if ( to.exists() && !to.isEmpty() && c==LEAVE)
        Throw(ex::filesystem::AlreadyThere,to.path().string());
    // To consider paths like "hari/shiva/"
    if( to.filename() == "." || to.filename() == "..")
        to.path( to.path() / filename());
    if ( to.path().has_parent_path() )
        Directory(to.parentpath().string()).create();
    fs::rename(m_name,to.m_name);
    path(to.path());
}

// Returns content size of Directories
// Throws fs::filesystem::NotThere, ex::Invalid
uintmax_t Directory::size(){
    if (!exists())
        Throw(ex::filesystem::NotThere,m_name.string());

    uintmax_t si = 0;
    std::vector<std::string> filelist = listrecursive(FILE);
    for(unsigned i=0;i<filelist.size();i++)
        si += File(filelist[i]).size();
    std::vector<std::string> directorylist = listrecursive(DIRECTORY);
    // Consider own folder size so adding 1 to directorylist
    // A folder size is 4096 bytes in my system
    // TODO check for windows
    si += (directorylist.size()+1) * 4096;
    return si;
}
// End of File Directory.cpp
