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
        // Derives from Error and says that there is no bottleneck
        class NoBottleneck: public Error {
            public:
                // construct the error object
                NoBottleneck(const std::string& o)
                    : Error("Bottleneck not found.", o) {
                    }
        };
        // Class ex::aggregate::AlreadyComplete
        // Derives from Error and says that download was already complete
        class AlreadyComplete: public Error {
            public:
                // construct the error object
                AlreadyComplete(const std::string& o)
                    : Error("Donwnload is Already Complete", o) {
                    }
        };
    };
};
#endif
// End File exAggregate.h
