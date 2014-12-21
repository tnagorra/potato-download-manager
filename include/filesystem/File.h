// File File.h
// Contains the File class.

#ifndef __CO_FILE__
#define __CO_FILE__
#include "filesystem/Node.h"
#include "filesystem/Directory.h"
#include <fstream>

// Let's be aware of Directory's existence
class Directory;

// Class File
// Represents a file, a filesytem node that can be written to
// or read from. Derives from Node.
class File : public Node {
    public:
        // State of the File Object.
        enum State {CLOSED,READ,WRITE,APPEND};
        // Mode of opening. (for us almost always will be binary)
        enum Mode {BINARY,TEXT};
    protected:
        // The standard library stream that represents the file
        std::fstream m_stream;

        // The mode in which it has been opened
        Mode m_mode;
        // The state which it is in
        State m_state;

        // Asserts that the filename is clean for portability.
        // That means the function does nothing if the filename is
        // portable, and throws ex::Invalid if its not.
        void assertClean();

        // Closes the opened file
        void close();

        // Closes the file if it is not already closed and it is not
        // in the State specified by 'stat'. Returns True in this case.
        // If it is not already closed but in State 'stat', then
        // returns false and does nothing.
        // Throws ex::Invalid if file is already closed.
        bool closeIfNot(State stat);

        // Opens the file for reading, writing or appending
        // Creates directories if necessary. Throws ex::Error if
        // opening fails.
        // NOTE: A file is opened only once if the mode is same,
        // so reopening a file with same state does nothing.
        void open(State state);

        // Used to copy n bytes of data from input stream to m_stream
        // If n is 0, then total input stream is copied
        void streamCopy(std::istream& in, uintmax_t n=0);
    public:
        // returns stream but risky
        std::fstream& ostream();

        // returns stream but risky
        std::fstream& istream();

        // Constructor of the object.
        // Throws ex::Not if given node not a file.
        File(const fs::path& name,Mode mode=BINARY);

        // Destructor
        ~File();

        // Returns the size of the File
        // Throws fs::filesystem_error
        uintmax_t size() const;

        // Removes the File
        // Returns False if file didn't exist in the first place,
        // else returns True
        // Throws fs::filesystem_error
        int remove();

        // Copies a File to another file
        // Throws fs::filesystem_error, ex::filesystem::AlreadyThere,
        // ex::filesystem::NotThere
        void copy(const fs::path& tos,Conflict c = LEAVE);

        // Moves a file
        // Throws fs::filesystem_error, ex::filesystem::AlreadyThere,
        // ex::filesystem::NotThere
        void move(const fs::path& to,Conflict c = LEAVE);

        // Writes an empty File
        // Throws ex::filesystem::AlreadyThere
        void write(Conflict c = LEAVE);

        // Writes a File with content data, B+T
        // Throws ex::filesystem::AlreadyThere
        void write(const std::string& data,Conflict c = LEAVE);

        // Writes a File with content data, B+T ??
        // Throws ex::filesystem::AlreadyThere
        void write(std::istream& data,Conflict c=LEAVE,uintmax_t n=0);

        // Appends to a existing File, B+T
        void append(const std::string& data);

        // Appends to an existing File contents of another File, B
        void append(File& to);

        // Appends to an existing File contents of another File, B
        void append(std::istream& data,uintmax_t n=0);

        // Returns the line string from an existing File, T
        // Throw ex::Error, ex::End, ex::filesystem::NotThere
        std::string readLine();

        // Returns a word string from an existing File, T
        // Throw ex::Error, ex::End, ex::filesystem::NotThere
        std::string readWord();
};

#endif
// End File File.h
