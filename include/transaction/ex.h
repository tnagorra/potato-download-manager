// File exDownload.h
// Contains exception classes for downloading-related errors,
// to be used by all parts of karmic for handling and reporting
// such kind of errors.

#ifndef DOWNLOAD_EXCEPTION_H
#define DOWNLOAD_EXCEPTION_H
#include "common/ex.h"

// Namesepace ex
// This is the namespace which contains all exception classes.
namespace ex {

// Namesepace ex::download
// This is the namespace which contains classes for downloading-
// related errors.
namespace download{

// Class ex::download::HostNotFound
// Says that the remote host could not be found.
class HostNotFound : public Error {
    public:
    HostNotFound(const std::string& hostname, const std::string& o)
        : Error("Server "+hostname+" not found.", o) {
    }
};

// Class ex::download::CouldNotConnect
// Says that the host couldn't be connected to.
class CouldNotConnect : public Error {
    public:
    CouldNotConnect(const std::string& detail, const std::string& o)
        : Error("Could not connect.\n"+detail, o) {
    }
};

// Class ex::download::ErrorSendingRequest
// Says that there was an error while sending a request to the host.
class ErrorSendingRequest : public Error {
    public:
    ErrorSendingRequest(const std::string& detail,const std::string& o)
        : Error("There was an error sending the HTTP request.\n"+
                detail, o) { }
};

// Class ex::download::BadResponse
// Says that the response sent by the remote host couldn't be
// understood.
class BadResponse : public Error {
    public:
    BadResponse(const std::string& o)
        : Error("The server sent a reply that could not be understood."
                , o) { }
};

// Class ex::download::Chunked
// Says that the file sent by the server is Chunked.
class Chunked: public Error {
    public:
    Chunked(const std::string& o)
        : Error("Transfer-Encoding is Chunked."
                , o) { }
};

// Class ex::download::BadResponse
// Says that the server replied with a non-favourable status code
// and the download process cannot be continued further. This
// exception could be thrown, for example, when a 500 Internal Server
// Error is received.
class BadStatusCode : public Error {
    public:
    BadStatusCode(const unsigned status_code, const std::string& o)
        : Error("The server replied with a status code of "+
                std::to_string(status_code),o) {
    }
};

// Class ex::download::ErrorReadingResponse
// Says that there was an error reading the response sent by the
// server, which may be due to various causes that I don't yet know of.
class ErrorReadingResponse : public Error {
    public:
    ErrorReadingResponse(const std::string& m, const std::string& o)
        : Error(
    "There was an error reading the response sent by the server.\n"+m,
    o) { }
};

// Class ex::download::BadProtocolScheme
// Says that the Protocol Scheme provided was not understood.
class BadProtocolScheme : public Error {
    public:
    BadProtocolScheme(const std::string& o)
        : Error("The specified protocol scheme is not supported",o) {}
    BadProtocolScheme(const std::string& o, const std::string& scheme)
        : Error("The protocol scheme "+scheme+" is not supported",o) {}

};


}; // end Namespace ex::download
}; // end Namespace ex

#endif // DOWNLOAD_EXCEPTION_H

// End File exDownload.h
