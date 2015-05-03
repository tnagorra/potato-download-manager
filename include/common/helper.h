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
#include <limits>
#include <cmath>
#include <ctime>
#include <cctype>
#include "common/ex.h"

// FOR colors

enum CC {BLACK,RED,GREEN,YELLOW,BLUE,PURPLE,TEAL,WHITE,GRAY};
#define COLOR(bold,fore,back) "\033["+std::to_string(bold)+";"+std::to_string(30+fore)+";"+std::to_string(40+back)+"m"
#define DISCOLOR "\033[0m"
#define DELETE "\033[A\033[2K"

#define SUCCESS COLOR(0,CC::GREEN,CC::BLACK)
#define NOTIFY COLOR(0,CC::BLUE,CC::BLACK)
#define WARNING COLOR(0,CC::YELLOW,CC::BLACK)
#define ERROR COLOR(0,CC::RED,CC::BLACK)

#define print(thing) std::cout<<thing<<std::endl
#define fancyprint(thing,color) std::cout << color << thing << DISCOLOR << std::endl
#define fancycout(thing,color) std::cout << color << thing << DISCOLOR

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

inline uintmax_t max(uintmax_t a, uintmax_t b){
    return a>b?a:b;
}

inline uintmax_t min(uintmax_t a, uintmax_t b){
    return a<b?a:b;
}

// Returns the md5 hashing of a text
std::string md5(std::string text);

// Returns a pretty name from a url
std::string prettify(std::string url);

// Remove the default port 80 from url 
std::string removeport80(std::string url);

// Returns a decoded url
std::string decodeUrl(std::string url);

// Return conversion of char to hex
int charToHex(char x);

// Helper to sort string according to numeric value
bool numerically(std::string const & a, std::string const & b);

// Helper to find if string is numeric
bool isNumeric(const std::string& s);

// Prints out a progressbar
std::string progressbar(float progress,std::string yes, std::string no,unsigned len=50);

#endif
