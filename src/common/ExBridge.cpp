#include "common/ExBridge.h"

// Constructor
ExBridge::ExBridge() : m_exNumber(0) {}

// Returns true if there is at least one logged exception.
bool ExBridge::fatal() {
    boost::shared_lock<boost::shared_mutex> m(m_mutex);
    return (m_exNumber!=0);
}

// Returns the number of exceptions that have been logged
uintmax_t ExBridge::number() {
    boost::shared_lock<boost::shared_mutex> m(m_mutex);
    return m_exNumber;
}

// Register an exception
void ExBridge::log(std::runtime_error& exc) {
    boost::upgrade_lock<boost::shared_mutex> lock(m_mutex);
    boost::upgrade_to_unique_lock<boost::shared_mutex> uLock(lock);
    m_exNumber++;
    m_exceptions.push_back(exc);
}

// Returns the exception vector
std::vector<std::runtime_error> ExBridge::all() {
    boost::shared_lock<boost::shared_mutex> m(m_mutex);
    return m_exceptions;
}

// Pops an exception from m_exceptions;
std::runtime_error ExBridge::pop() {
    boost::upgrade_lock<boost::shared_mutex> lock(m_mutex);
    boost::upgrade_to_unique_lock<boost::shared_mutex> uLock(lock);
    if (m_exNumber==0)
        return std::runtime_error("No exceptions");
    m_exNumber--;
    std::runtime_error ret = m_exceptions.back();
    m_exceptions.pop_back();
    return ret;
}
