// File ex.h
// Contains base exception classes and helper macros
// to be used by all parts of karmic for error reporting
// and handling.

#ifndef __CO_EX__
#define __CO_EX__
#include<iostream>
#include<string>

// ▸ ✖ ✕ ✘ ➟ ➡ ➤ ➲ ➱ ➾

// The following macros are helpers for easy exception-throwing.
// You don't need to remember how many arguments an exception-
// constructor takes, just use Throw()

// Look at definition of Throw(...)
#define GET_MACRO(_1,_2,_3,NAME,...) NAME

// Throw a three-argument exception, where the first two arguments
// are the arguments to the exception constructor and the last one
// is the occurance string, a string describing where (file, line,
// function) where the error occured.
#define Throw3(name,arg1,arg2) throw  name(arg1,arg2,"✘ "+\
        std::string(__FILE__)+":"+std::to_string(__LINE__)+\
        " inside function \""+std::string(__FUNCTION__)+"\"")

// Throw a two-argument exception, where the first argument is as
// necessary for the exception and the second is the occurance string.
#define Throw2(name,arg) throw  name(arg, "✘ "+std::string\
        (__FILE__)+":"+std::to_string(__LINE__)+\
        " inside function \""+std::string(__FUNCTION__)+"\"")

// Throw a single-argument exception, whose only argument is the
// occurance string.
#define Throw1(name) throw name("✘ "+std::string(__FILE__)+\
        ":"+std::to_string(__LINE__)+" inside function \""+\
        std::string(__FUNCTION__)+"\"")

// The entry point. Decides which macro to call using GET_MACRO() and
// calls it with the necessary arguments.
#define Throw(...) GET_MACRO(__VA_ARGS__, Throw3, Throw2, Throw1)\
        (__VA_ARGS__)

// Namesepace ex
// This is the namespace which contains all exception classes.
namespace ex{

    // Class ex::Error
    // This class derives from the standard c++ exception class, and
    // holds the error message and occurance string. The occurance
    // string is the locality information of the error.
    class Error: public std::exception {

        protected:
            std::string msg;        // The error message
            std::string occurance;  // The occurance string

        public:
            // construct the error object
            Error(const std::string& s, const std::string& o)
                : msg(s), occurance(o) { }

            // return a nice message containing the error message
            // and occurance.
            const char* what() const throw() {
                std::string str;
                if (occurance.length() > 0) str += occurance + "\n";
                str += "Error: "+msg+"\n";
                return str.c_str();
            }
    };

    // Class ex::Invalid
    // Derives from Class Error and describes something as invalid.
    class Invalid : public Error {

        public:
            // construct the error object, saying that something of
            // 'type' kind, with name 'name', is invalid.
            Invalid(const std::string& type, const std::string& name,
                    const std::string& o)
                : Error(type+" \""+name+"\" is invalid.", o) { }

            // construct the error object, saying that 'type'
            // is invalid.
            Invalid(const std::string& type, const std::string& o)
                : Error("\""+type+"\" is invalid.", o) { }
    };

    // Class ex::Not
    // Derives from Class Error and says that something is not
    // something else.
    class Not : public Error {

        public:
            // construct the error object, saying that 'name' isn't
            // something of 'type' kind, when it should.
            Not(const std::string& type, const std::string& name,
                   const std::string& o)
                : Error("\""+name+"\" is not a \""+type+"\".", o) { }

            // construct the error object, saying that some unnamed
            // thing is not of the 'type' kind, when it should.
            Not(const std::string& type, const std::string& o)
                : Error("Not a \""+type+"\".", o) { }
    };

    // Class ex::ZeroLength
    // Derives from Error and says that something has a zero length,
    // meaning there's nothing there.
    class ZeroLength:public Error{

        public:
            // construct the object, saying what it should.
            ZeroLength(const std::string& name, const std::string& o)
                : Error("\""+name+"\" is of zero length.", o) { }
    };
};

#endif

// End File ex.h
