#include <typeinfo>
#include <iostream>
#include <boost/asio.hpp>
#include "common/ex.h"
#include "transaction/ex.h"
#include "transaction/Range.h"
#include "transaction/RemoteDataHttp.h"
#include "transaction/HttpTransaction.h"

void reader(boost::asio::streambuf& buf) {
    std::cout<<&buf;
}

int main(int argc, char* argv[]) try {
    RemoteData* rd = RemoteData::factory(argv[1]);
    boost::function<void (boost::asio::streambuf&)> rdr;
    rdr = reader;
    RemoteDataHttp* fku = static_cast<RemoteDataHttp*>(rd);
    switch (rd->scheme()) {
        case RemoteData::Protocol::https: {
            HttpsTransaction* remTran = new HttpsTransaction(fku);
            remTran->registerReader(rdr);
            remTran->start();
            while (!remTran->complete());
            break;
        }
        break;
        case RemoteData::Protocol::http: {
            fku->method("GET");
            HttppTransaction* remTran = new HttppTransaction(fku);
            remTran->registerReader(rdr);
            remTran->start();
            while(!remTran->complete());
            break; }
        default:
            std::cout<<"none\n";
            return 1;
            break;
    }
    return 0;
} catch (ex::Error& ex) {
    std::cout<<ex.what()<<std::endl;
}
