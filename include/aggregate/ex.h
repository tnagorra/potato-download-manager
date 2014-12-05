// File exFile.h
// Contains exception classes for aggreagate-related errors,
// to be used by all parts of karmic for handling and reporting
// such kind of errors.

#ifndef __CO_EX_AGGREGATE__
#define __CO_EX_AGGREGATE__
#include"common/ex.h"

// Namesepace ex
// This is the namespace which contains all exception classes.
namespace ex{

    // Namesepace ex::aggreagate
    // This is the namespace which contains classes for aggregate-
    // related errors.
    namespace aggregate{

        // Class ex::aggregate::NoBottleneck
        // Derives from Error and says that the end of a file has
        // been reached unexpectedly.
        class NoBottleneck: public Error {
            public:
                // construct the error object, saying that the end of
                // file 'name' has been reached.
                NoBottleneck(const std::string& name, const std::string& o)
                    : Error("Bottleneck "+name+" not found.", o) {
                    }
        };
    };
};
#endif
    // End File exAggregate.h
