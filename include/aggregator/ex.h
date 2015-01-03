// File ex.h
// Contains exception classes for aggreagator-related errors,
// to be used by all parts of karmic for handling and reporting
// such kind of errors.

#ifndef __CO_EX_AGGREGATOR__
#define __CO_EX_AGGREGATOR__
#include"common/ex.h"

// Namesepace ex
// This is the namespace which contains all exception classes.
namespace ex{

    // Namesepace ex::aggregator
    // This is the namespace which contains classes for aggreagator-
    // related errors.
    namespace aggregator{

        class NoBottleneck: public Error {
            public:
                NoBottleneck(const std::string& o)
                    : Error("No bottleneck found.",o) { }
        };

        class NonResumable: public Error {
            public:
                NonResumable(const std::string& o)
                    :Error("Non resumable download",o) { }
        };
    };
};

#endif
// End File exFile.h
