// File RemoteDataHttp.cpp
// Contains the implementation for the class RemoteDataHttp

#include "transaction/RemoteDataHttp.h"

// Constructor
RemoteDataHttp::RemoteDataHttp(const std::string& fullUri,
        const std::string& method_,
        std::map<std::string, std::string> headers_,
        const std::string& body_)
    : RemoteData(fullUri), m_method(method_), m_headers(headers_),
    m_body(body_) { }

// Getters

// Returns the method
std::string RemoteDataHttp::method() const {
    return m_method;
}

// Returns the headers
std::map<std::string,std::string> RemoteDataHttp::headers() const {
    return m_headers;
}

// Returns the value of a specific header
std::string RemoteDataHttp::header(const std::string& head) const {
    std::map<std::string,std::string>::const_iterator it;
    it = m_headers.find(boost::to_lower_copy(head));
    if (it==m_headers.end())
        return std::string("");
    return it->second;
}

// Returns the value of m_body
std::string RemoteDataHttp::body() const {
    return m_body;
}

// Setters

// Sets the http request method
// TODO check validity?
void RemoteDataHttp::method(const std::string& meth) {
    m_method = meth;
}

// Sets the body
void RemoteDataHttp::body(const std::string& bdy) {
    m_body = bdy;
}

// Sets the headers map
void RemoteDataHttp::headers(const std::map<std::string,std::string>&
    hdrs) {
    m_headers = hdrs;
}

// Sets a certain header
void RemoteDataHttp::addHeader(const std::string& headr,
                const std::string& value) {
    m_headers[boost::to_lower_copy(headr)] = value;
}

// Remove a certain header
void RemoteDataHttp::removeHeader(const std::string& headr) {
    m_headers.erase(boost::to_lower_copy(headr));
}

// End File RemoteDataHttp.cpp
