// File RemoteData.h
// Contains the class RemoteData

#ifndef __CO_REMOTEDATA__
#define __CO_REMOTEDATA__
#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <cctype>
#include <boost/algorithm/string.hpp>
#include "common/ex.h"

class RemoteDataHttp;
// Class RemoteData
// This class represents a TCP connection to a remote server.
// The RemoteData class is an base class for other
// classes to derive from, one for a protocol type each.
class RemoteData {

    public:
        enum class Protocol { http, https, ftp, none };
        enum class Partial { yes, no, unknown };

    protected:
        // Full URI of the Resource
        std::string m_fullUrl;
        // Protocol (scheme)
        Protocol m_scheme;
        // Server name
        std::string m_server;
        // Resource path, if any
        std::string m_path;
        // filename
        std::string m_filename;
    private:
        // Are partial requests allowed?
        Partial m_canPartial;

    public:
        // The factory method. It takes in a fullUrl and returns a
        // pointer to a  RemoteData Object of the appropriate type.
        static RemoteData* factory(const std::string& fullUrl);

        // Constructor
        RemoteData(const std::string& fullUrl);
        // Destructor
        virtual ~RemoteData() {}

        // Getters
        // Returns the fullUrl
        std::string fullUrl() const;
        // Returns the scheme
        Protocol scheme() const;
        // Returns the scheme as a string
        const char* schemeCStr() const;
        // Returns the servername
        std::string server() const;
        // Returns the path
        std::string path() const;

        // Returns whether partial downloads are allowed
        Partial canPartial() const;
        // Returns the download filename
        std::string filename() const;

        void filename(const std::string& file_name);

        // Return the Protocol:: enum from a string
        Protocol schemeFromString(const std::string& sch) const;

        // Setters
        // Sets the fullUrl (and does a lot else)
        void fullUrl(const std::string& fUrl);
        // Sets whether partial downloads are allowed
        void canPartial(Partial can_partial);
};

#endif
// End File RemoteData.h
