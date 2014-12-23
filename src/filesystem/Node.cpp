// File Node.cpp
// Cointains implementation for class Node

#include "filesystem/Node.h"

// Asserts that the filename is clean for portability.
// That means the function checks if the filename is
// portable, and throws ex::Invalid if its not.
void Node::assertClean(){
    if(m_name.string().length() <= 0)
        Throw(ex::ZeroLength,"Node name");

    m_name.make_preferred();
    m_name.normalize();

    /* TODO
    // swap protable_name with native
    // Here sanitization doesn't support whitespaces?
    // Check recursively for portable compability
    fs::path check = m_name;
    while(check.string().length() > 0){
    // Isn't compatible for Windows
    // Because / gives some error!!
    if( check.string() == "/")
    break;
    if( !fs::portable_name(check.filename().string()))
    Throw(ex::Invalid,"Name",check.filename().string());
    check = check.parent_path();
    }
    */
}

// Constructor, takes a string for filename.
Node::Node(const fs::path& name) : m_name(name) {
    assertClean();
}

// Sets a different path
void Node::path(const fs::path& name) {
    m_name = name;
    assertClean();
}
/*
fs::path Node::presuf(const fs::path& prefix, const fs::path& suffix) const {
    return parentpath() / prefix + filename() + suffix;
}
*/

// Returns new path in case of conflict
fs::path Node::newpath() const {
    std::string npath = m_name.c_str();
    if( Node(npath).exists() ){
        unsigned i = 1;
        while(1){
            npath = (m_name.parent_path()/m_name.stem()).c_str()+std::string(".")+std::to_string(i)+m_name.extension().c_str();
            if (!Node(npath).exists())
                break;
            i++;
        }
    }
    return npath;
}

// Returns the path
fs::path Node::path() const {
    return m_name;
}

// Returns the absolute path
fs::path Node::absolutepath() const {
    return complete(m_name,fs::initial_path());
}

// Returns only the filename in the path
fs::path Node::filename() const {
    return m_name.filename();
}

// Returns the parentpath
// Throws ex::Invalid
fs::path Node::parentpath() const {
    if (!m_name.has_parent_path())
        Throw(ex::Invalid,"Parent path");
    return m_name.parent_path();
}

// Returns what type the Node is
Node::Type Node::what() const {
    if( !exists() )
        Throw(ex::filesystem::NotThere,m_name.string());
    if(fs::is_directory(m_name))
        return DIRECTORY;
    else if (fs::is_regular_file(m_name))
        return FILE;
    else
        return SOMETHING;
}

// Returns if the Node exists
bool Node::exists() const {
    return fs::exists(m_name);
}

// Returns diskspace
// Throws ex::Invalid, ex::ZeroLength()
uintmax_t Node::diskspace(Disk m) const {
    // Get check name and goes on until finding the node which exists
    fs::path checkName = m_name;
    while( !fs::exists(checkName)){
        checkName = checkName.parent_path();
        if(checkName.string().length()==0)
            Throw(ex::ZeroLength,"Path");
    }
    fs::space_info sp = fs::space(checkName);
    switch(m){
        case CAPACITY: return sp.capacity;
        case AVAILABE: return sp.available;
        case FREE: return sp.free;
        default: Throw(ex::Invalid,"Mode");
    }
}

// End of File Node.cpp
