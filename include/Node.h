// File Node.h
// Contains the Node class.
#ifndef __CO_OBJECT__
#define __CO_OBJECT__
#include <boost/filesystem.hpp>
#include "exFile.h"

namespace fs = boost::filesystem;

// Class Node
// Represents a node of the filesystem. It could be either a
// file or a directory, so this class provides the interface and
// some functionality common to both files and directories.
class Node{
    public:
        // Things that may be done in case of a name conflict:
        // Either force the action, or do nothing.
        enum Conflict {FORCE,LEAVE};
        // Types of disk space information. CAPACITY is the total
        // storage capacity. FREE and AVAILABLE are similar, but
        // I don't know the difference.
        enum Disk {FREE,AVAILABE,CAPACITY};
        // Type of the node. It is nothing if it doesn't exist.
        // If it does, it is either a file or directory. It is
        // something if it none of these.
        enum Type {SOMETHING,FILE,DIRECTORY};
    protected:
        // The full filename (path+name) of the node
        fs::path m_name;

        // Asserts that the filename is clean for portability.
        // That means the function checks if the filename is
        // portable, and throws ex::Invalid if its not.
        virtual void assertClean();
    public:
        // Constructor, takes a string for filename
        Node(const fs::path& name);

        // Sets a different path
        virtual void path(const fs::path& name);

        // Returns the path
        fs::path path() const;

        // Returns the absolute path
        fs::path absolutepath() const;

        // Returns the parent path
        // Throws ex::Invalid
        fs::path parentpath() const;

        // Returns only the filename in the path
        fs::path filename() const;

        // Returns what type the Node is
        Type what() const;

        // Returns if the Node exists
        bool exists() const ;

        // Returns diskspace
        // Throws ex::Invalid, ex::ZeroLength()
        uintmax_t diskspace(Disk m = AVAILABE) const;
};

#endif
// End File Node.h
