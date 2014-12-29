// File Range.h
// Contains the implementation for the Range Class

#include "transaction/Range.h"

// Constructor. (0,0) is the default, means
// entire file.
// Throws ex::Invalid
Range::Range(uintmax_t ub, uintmax_t lb) {
    update(ub,lb);
}

Range::Range(const Range& r){
    boost::shared_lock<boost::shared_mutex> m(r.m_mutex);
    update(r.m_ub,r.m_lb);
}

void Range::operator=(const Range& r){
    boost::shared_lock<boost::shared_mutex> m(r.m_mutex);
    update(r.m_ub,r.m_lb);
}

void Range::check(){
    if(m_ub < m_lb)
        Throw(ex::Invalid, "Range", "("+std::to_string(m_ub)+","+std::to_string(m_lb)+")" );
}

// Updates the range value
// Throws ex::Invalid
void Range::update(uintmax_t ub,uintmax_t lb){
    boost::upgrade_lock<boost::shared_mutex> lock(m_mutex);
    boost::upgrade_to_unique_lock<boost::shared_mutex> uniqueLock(lock);
    m_ub = ub;
    m_lb = lb;
    check();
}


uintmax_t Range::ub(uintmax_t u){
    boost::upgrade_lock<boost::shared_mutex> lock(m_mutex);
    boost::upgrade_to_unique_lock<boost::shared_mutex> uniqueLock(lock);
    m_ub= u;
    check();
    return m_ub;
}


uintmax_t Range::lb(uintmax_t l){
    boost::upgrade_lock<boost::shared_mutex> lock(m_mutex);
    boost::upgrade_to_unique_lock<boost::shared_mutex> uniqueLock(lock);
    m_lb = l;
    check();
    return m_lb;
}

// Returns the lower bound of the range
uintmax_t Range::lb() const {
    boost::shared_lock<boost::shared_mutex> m(m_mutex);
    return m_lb;
}

// Returns the upper bound of the range
uintmax_t Range::ub() const {
    boost::shared_lock<boost::shared_mutex> m(m_mutex);
    return m_ub;
}

uintmax_t Range::size() const {
    boost::shared_lock<boost::shared_mutex> m(m_mutex);
    return m_ub-m_lb;
}

// Returns if uninitialized
bool Range::uninitialized() const {
    boost::shared_lock<boost::shared_mutex> m(m_mutex);
    return m_ub == 0 && m_lb == 0;
}

// End File Range.cpp
