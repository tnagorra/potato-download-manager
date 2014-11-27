// File Range.h
// Contains the implementation for the Range Class

#include "Range.h"

// Constructor. (0,0) is the default, means
// entire file.
// Throws ex::Invalid
Range::Range(uintmax_t ub, uintmax_t lb) {
    update(ub,lb);
}

// Updates the range value
// Throws ex::Invalid
void Range::update(uintmax_t ub,uintmax_t lb){
    if( ub < lb)
        Throw(ex::Invalid, "Range", "("+std::to_string(lb)+","+std::to_string(ub)+")" );
    m_lb = lb;
    m_ub = ub;
}

// Returns the size of the range
uintmax_t Range::size() const {
    return m_ub-m_lb;
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
