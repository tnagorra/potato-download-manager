// File helper.h
// Contains helper functions

#ifndef __CO_HELPER__
#define __CO_HELPER__
extern "C" {
#include <openssl/md5.h>
}
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <climits>
#include <cmath>
#include <ctime>

// Infinity ∞ unicode

// Returns the time in formatted form
std::string formatTime(uintmax_t time) ;

// Returns the bytes in formatted form
std::string formatByte(uintmax_t byte);

// Retuns a random alphanumeric string
std::string randomString(uintmax_t len=32);

// Returns a number rounded to n digits after decimal points
inline double round(double num,uintmax_t n){
    unsigned tmp = pow(10,2);
    return std::round(num*tmp)/tmp;
}

// Returns the md5 hashing of a text
std::string md5(std::string text);

// Helper to sort filenames according to numerical value
/*
   inline bool numerically(std::string const & a, std::string const & b){
   return stoi(a) < stoi(b);
   }
   */

#endif
