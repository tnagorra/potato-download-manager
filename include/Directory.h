// File Directory.h
// Contains the Directory Class

#ifndef __CO_DIRECTORY__
#define __CO_DIRECTORY__
#include"Node.h"
#include"File.h"

// Class Directory
// Represents a directory, which is a filesystem node with can
// contain other nodes (files or directories)
class Directory : public Node {
    protected:
        // Asserts that the filename is clean for portability.
        // That means the function does nothing if the filename is
        // portable, and throws ex::Invalid if its not.
        void assertClean();
    public:
        // Constructor
        // Throws ex::Not
        Directory(const fs::path& name);

        // Lists files or directories and returns the vector of string
        // Throws fs::filesystem_error, ex::file::NotThere
        std::vector<std::string> list(Type type = FILE, bool onlyfilename = false);

        // Lists files or directoires recursively and returns the vector of string
        // Throws fs::filesystem_error, ex::file::NotThere
        std::vector<std::string> listrecursive(Type type = FILE, bool onlyfilename = false);

        // Returns if the Directory is empty
        // Throws fs::filesystem_error
        bool isEmpty() const;

        // Creates a Directory
        // Throws fs::filesystem_error
        void create();

         // removes Directories recursively/nonrecursively
        // Returns the number of files removed.
        // Throws fs::filesystem_error
        int remove(Conflict c = LEAVE);

        // Copies Directories
        // Throws fs::filesystem_error, ex::file::AlreadyThere,
        // ex::file::NotThere, ex::Invalid
        void copy(const fs::path& tos,Conflict c = LEAVE);

        // Moves a Directory
        // Throws fs::filesystem_error, ex::file::AlreadyThere,
        // and ex::file::NotThere
        void move(const fs::path& tos, Conflict c = LEAVE);

       // Returns content size of Directories
        // Throws fs::file::NotThere, ex::Invalid
        uintmax_t size();
};

#endif
// End File Directory.h
