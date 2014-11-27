// File RemoteData.cpp
// Contains the implementation for the class RemoteData

#include "RemoteData.h"
// Factory needs the children to be fully declared
#include "RemoteDataHttp.h"

// The factory method. It takes in a fullUrl and returns a
// pointer to a RemoteData Object of the appropriate type.
// **static**
RemoteData* RemoteData::factory(const std::string& fUrl) {
    RemoteData* decider = new RemoteData(fUrl);
    switch (decider->scheme()) {
        case Protocol::http:
        case Protocol::https: {
            RemoteDataHttp* product = new RemoteDataHttp(fUrl);
            return product;
            break; }
        default:
            return decider;
    }
}

// Constructor
RemoteData::RemoteData(const std::string& fUrl) {
    fullUrl(fUrl);
}

// Getters
// Returns the fullUrl
std::string RemoteData::fullUrl() const {
    return m_fullUrl;
}

// Returns the scheme
RemoteData::Protocol RemoteData::scheme() const {
    return m_scheme;
}

// Returns the scheme as a c string
const char* RemoteData::schemeCStr() const {
    switch (m_scheme) {
        case Protocol::http:
            return "http";
        case Protocol::https:
            return "https";
        case Protocol::ftp:
            return "ftp";
        case Protocol::none:
            return "none";
        default:
            return NULL;
    }
}

// Returns the servername
std::string RemoteData::server() const {
    return m_server;
}

// Returns the path
std::string RemoteData::path() const {
    return m_path;
}

// Returns the prettyName
std::string RemoteData::prettyName() const {
    return m_prettyName;
}

// Return the Protocol:: enum from a string
RemoteData::Protocol RemoteData::schemeFromString(const std::string&
        sch) const {
    std::string sche = boost::to_lower_copy(sch);
    if (sche=="http")
        return Protocol::http;
    else if (sche=="https")
        return Protocol::https;
    else if (sche=="ftp")
        return Protocol::ftp;
    else
        return Protocol::none;
}

// Setters

// Set the Full Url of the resource. This method needs to do
// a lot of work, like checking the validity of the url, separating
// the components and url-encoding special characters.
// TODO This method has become very dense! Separate it to a class?
void RemoteData::fullUrl(const std::string& fUrl) {
    m_fullUrl = fUrl;
    m_scheme = Protocol::none;
    m_path = "";
    m_server = "";

    size_t first_slash = m_fullUrl.find('/');
    std::string url;    // fullUrl minus the scheme

    // If there's no slash, entire string is 'server' part
    if (first_slash==std::string::npos) {
        url = m_fullUrl;
        m_path = "/";
        m_server = m_fullUrl;

    // If a slash is the first character, URI is invalid.
    } else if (first_slash==0) {
        Throw(ex::Invalid, "The URL", m_fullUrl);

    // If the first slash is a part of :// then the left side
    // is a scheme specifier, and the rest is the url
    } else if (m_fullUrl[first_slash-1]==':' &&
            m_fullUrl[first_slash+1]=='/') {
        std::string scheme = m_fullUrl.substr(0, first_slash-1);
        boost::algorithm::to_lower(scheme);

        // Set the m_scheme
        if (schemeFromString(scheme)!=Protocol::none)
            m_scheme = schemeFromString(scheme);
        else
            Throw(ex::Invalid, "Protocol", scheme);

        // Extract the url
        url = m_fullUrl.substr(first_slash+2,
                m_fullUrl.size()-first_slash-2);

    // At this point, the part before the first slash is severname
    // and the part after that is the path
    } else {
        url = m_fullUrl;
        m_server = m_fullUrl.substr(0,first_slash);
        m_path = m_fullUrl.substr(first_slash,
                m_fullUrl.size()-first_slash);
    }

    // Empty!
    if (url.size()==0)
        Throw(ex::ZeroLength, "The URL");

    // If the scheme has not been set to anything, assume http
    if (m_scheme==Protocol::none)
        m_scheme=Protocol::http;

    // If the scheme has been separated, but the server and path
    // have not been.
    if (m_path=="" && m_server=="") {
        first_slash = url.find('/');
        if (first_slash==std::string::npos) {
            m_path = "/";
            m_server = url;
        } else {
            m_server = url.substr(0,first_slash);
            m_path = url.substr(first_slash,
                url.size()-first_slash);
        }
    }

    // Everything has been separated. Sanitization now.

    // Check if a disallowed character is in m_server.
    for (std::string::iterator it=m_server.begin(); it!=m_server.end();
            it++)
        if (!(isalnum(*it) || (*it)=='.' || (*it)=='-'))
            Throw(ex::Invalid,"Server",m_server);

    // Percent-encode any special characters in the path string.
    std::string noEncode = "$+/:;=?@%&.";
    std::vector<uint8_t> bytes(m_path.begin(), m_path.end());
    std::stringstream newPath;
    for (std::vector<uint8_t>::iterator it=bytes.begin();
            it!=bytes.end(); it++)
        if (!isalnum(char(*it))
                && noEncode.find(char(*it))==std::string::npos)
            newPath<<'%'<<std::hex<<std::uppercase<<(int)*it;
        else
            newPath<<(char)*it;
    m_path = newPath.str();
}

// End File RemoteData.cpp
