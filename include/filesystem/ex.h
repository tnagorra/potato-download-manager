// File exFile.h
// Contains exception classes for filesystem-related errors,
// to be used by all parts of karmic for handling and reporting
// such kind of errors.

#ifndef __CO_EX_FILE__
#define __CO_EX_FILE__
#include"common/ex.h"

// Namesepace ex
// This is the namespace which contains all exception classes.
namespace ex{

    // Namesepace ex::file
    // This is the namespace which contains classes for filesystem-
    // related errors.
    namespace filesystem{


        class NotFile : public Not {

            public:
                // construct the error object, saying that 'name' isn't
                // something of 'type' kind, when it should.
                NotFile(const std::string& name,  const std::string& o)
                    : Not("\""+name+"\" is not a file.", o) { }

                // construct the error object, saying that some unnamed
                // thing is not of the 'type' kind, when it should.
                NotFile(const std::string& o)
                    : Not("Not a file.", o) { }
        };


        class NotDirectory: public Not {

            public:
                // construct the error object, saying that 'name' isn't
                // something of 'type' kind, when it should.
                NotDirectory(const std::string& name,  const std::string& o)
                    : Not("\""+name+"\" is not a directory.", o) { }

                // construct the error object, saying that some unnamed
                // thing is not of the 'type' kind, when it should.
                NotDirectory(const std::string& o)
                    : Not("Not a directory.", o) { }
        };

        // Class ex::filesystem::NotThere
        // Derives from Error and says that a file or directory is
        // not there, when it should.
        class NotThere : public Error {

            public:
                // construct the error object, saying that 'name'
                // is not there.
                NotThere(const std::string& name,const std::string& o)
                    : Error("\""+name+"\" is not there.", o) { }
        };

        // Class ex::filesystem::AlreadyThere
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

        // Class ex::filesystem::End
        // Derives from Error and says that the end of a file has
        // been reached unexpectedly.
        class End : public Error {
            public:
                // construct the error object, saying that the end of
                // file 'name' has been reached.
                End(const std::string& name, const std::string& o)
                    : Error("EOF of file \""+name+"\" reached.",o) { }
        };

        // Class ex::filesystem::ZeroSize
        // Derives from Error and says that the file is of zero size
        class ZeroSize: public Error {
            public:
                // construct the error object, saying that the end of
                // file 'name' has been reached.
                ZeroSize(const std::string& name, const std::string& o)
                    : Error("File\""+name+ "\"seems to have zero size.",o) { }
                ZeroSize(const std::string& o)
                    : Error("File seems to have zero size.",o) { }
        };

        // Class ex::filesystem::ZeroSize
        // Derives from Error and says that the file is of zero size
        class NonZeroSize: public Error {
            public:
                // construct the error object, saying that the end of
                // file 'name' has been reached.
                NonZeroSize(const std::string& name, const std::string& o)
                    : Error("File\""+name+ "\"seems to have non zero size.",o) { }
                NonZeroSize(const std::string& o)
                    : Error("File seems to have non zero size.",o) { }
        };
    };
};

#endif
// End File exFile.h
