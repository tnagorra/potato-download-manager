// File exFile.h
// Contains exception classes for filesystem-related errors,
// to be used by all parts of karmic for handling and reporting
// such kind of errors.

#ifndef __CO_EX_FILE__
#define __CO_EX_FILE__
#include"ex.h"

// Namesepace ex
// This is the namespace which contains all exception classes.
namespace ex{

    // Namesepace ex::file
    // This is the namespace which contains classes for filesystem-
    // related errors.
    namespace file{

        // Class ex::file::NotThere
        // Derives from Error and says that a file or directory is
        // not there, when it should.
        class NotThere : public Error {

            public:
                // construct the error object, saying that 'name'
                // is not there.
                NotThere(const std::string& name,const std::string& o)
                    : Error("\""+name+"\" is not there.", o) { }
        };

        // Class ex::file::AlreadyThere
        // Derives from Error and says that a file or directory is
        // already there, when it should not have been.
        class AlreadyThere : public Error {

            public:
                // construct the error object, saying that 'name'
                // is already there.
                AlreadyThere(const std::string& name,
                        const std::string& o)
                    : Error("\""+name+"\" is already there.",o) { }
        };

        // Class ex::file::End
        // Derives from Error and says that the end of a file has
        // been reached unexpectedly.
        class End : public Error {
            public:
                 // construct the error object, saying that the end of
                 // file 'name' has been reached.
                End(const std::string& name, const std::string& o)
                    : Error("EOF of file \""+name+"\" reached.",o) { }
        };
    };
};

#endif
// End File exFile.h
