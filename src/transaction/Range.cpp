// File Range.h
// Contains the implementation for the Range Class

#include "transaction/Range.h"

// Constructor. (0,0) is the default, means
// entire file.
// Throws ex::Invalid
Range::Range(uintmax_t ub, uintmax_t lb) {
    update(ub,lb);
}

void Range::check(){
    if(m_ub < m_lb)
        Throw(ex::Invalid, "Range", "("+std::to_string(m_ub)+","+std::to_string(m_lb)+")" );
}

// Updates the range value
// Throws ex::Invalid
void Range::update(uintmax_t ub,uintmax_t lb){
    m_lb = lb;
    m_ub = ub;
    check();
}

// Returns the size of the range
uintmax_t Range::size() const {
    return m_ub-m_lb;
}

uintmax_t Range::ub(uintmax_t u){
    m_ub = u;
    check();
    return m_ub;
}

uintmax_t Range::lb(uintmax_t l){
    m_lb = l;
    check();
    return m_lb;
}

// Returns the lower bound of the range
uintmax_t Range::lb() const {
    return m_lb;
}

// Returns the upper bound of the range
uintmax_t Range::ub() const {
    return m_ub;
}

// Returns if uninitialized
bool Range::uninitialized() const {
    return m_ub == 0 && m_lb == 0;
}

// End File Range.cpp
