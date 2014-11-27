// File RemoteDataHttp.h
// Contains the class RemoteDataHttp

#ifndef __CO_REMOTEDATAHTTP__
#define __CO_REMOTEDATAHTTP__
#include <iostream>
#include <string>
#include <map>
#include "transaction/RemoteData.h"

// Class RemoteDataHttp
// This class represents a TCP-HTTP connection to a remote server.
// It is a specialization of the base class RemoteData for the
// http protocol
class RemoteDataHttp : public RemoteData {

    private:
        // HTTP method (?)
        std::string m_method;

        // HTTP Headers to add to the request
        std::map<std::string,std::string> m_headers;

        // The request body, if required
        std::string m_body;

    public:
        // Constructor
        RemoteDataHttp(const std::string& fullUri,
                const std::string& method_ = "GET",
                std::map<std::string, std::string> headers_ =
                    std::map<std::string, std::string>(),
                const std::string& body_ = "");

        // Getters
        // Returns the method
        std::string method() const;

        // Returns the headers
        std::map<std::string,std::string> headers() const;

        // Returns the value of a specific header
        std::string header(const std::string& head) const;

        // Returns the value of m_body
        std::string body() const;

        // Setters
        // Sets the http request method
        void method(const std::string& meth);

        // Sets the body
        void body(const std::string& bdy);

        // Sets the headers map
        void headers(const std::map<std::string,std::string>& hdrs);

        // Sets a certain header
        void addHeader(const std::string& headr,
                const std::string& value);

        // Remove a certain header
        void removeHeader(const std::string& headr);
};

#endif
// End File RemoteDataHttp.h
