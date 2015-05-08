#ifndef __CO_EXBRIDGE__
#define __CO_EXBRIDGE__

#include <vector>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <stdexcept>

#include "common/ex.h"

// Class ExBridge. Used to transport exceptions from one thread to
// another. Exceptions generated in worker threads need to be
// transported to main threads, and they do it by using a shared
// instance of ExBridge. ExBridge is meant to be shared by multiple
// threads, one of which listens for exceptions and the rest log
// exceptions.
class ExBridge {
    private:
        // Number of exceptions in the log.
        uintmax_t m_exNumber;
        // Vector to contain exceptions that have been thrown.
        std::vector<std::runtime_error> m_exceptions;
        // Mutex for internal locking
        mutable boost::shared_mutex m_mutex;

    public:
        // Constructor
        ExBridge();
        // Returns true if there is at least one logged exception.
        bool fatal();
        // Returns the number of exceptions that have been logged
        uintmax_t number();
        // Register an exception
        void log(std::runtime_error& exc);
        // Returns the exception vector
        std::vector<std::runtime_error> all();
        // Pops an exception from m_exceptions;
        std::runtime_error pop();
};

#endif
