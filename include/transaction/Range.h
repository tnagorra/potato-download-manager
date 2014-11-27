// File Range.h
// Contains the Range Class

#ifndef __CO_RANGE__
#define __CO_RANGE__
#include<iostream>
#include "ex.h"

// Class Range
// Represents a range or chunk of the file being downloaded.
// The chunk is characterized by a lower bound (starting value)
// and an upper bound (ending value). The default value of
// the range as implied by the constructor argument-list is
// (0,0), which signifies the entire file. Any other zero-length
// range signifies an empty chunk.
class Range {

    private:
        uintmax_t m_lb;
        uintmax_t m_ub;

    public:
        // Constructor. (0,0) is the default, means
        // entire file.
        // Throws ex::Invalid
        Range(uintmax_t ub=0, uintmax_t lb=0);

        // Updates the range value
        // Throws ex::Invalid
        void update(uintmax_t ub,uintmax_t lb);

        // Returns the size of the range
        uintmax_t size() const;

        // Returns the lower bound of the range
        uintmax_t lb() const;

        // Returns the upper bound of the range
        uintmax_t ub() const;

        // Returns if uninitialized
        bool uninitialized() const;
};

#endif
// End File Range.h
