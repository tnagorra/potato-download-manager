#include <typeinfo>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>
#include "common/ex.h"
#include "transaction/ex.h"
#include "transaction/Range.h"
#include "transaction/RemoteDataHttp.h"
#include "transaction/HttpTransaction.h"

void reader(boost::asio::streambuf& buf) {
    std::cout<<&buf;
}

int main(int argc, char* argv[]) try {
    boost::function<void (boost::asio::streambuf&)> rdr;
    rdr = reader;

    Range r;
    if (argc==3)
        r.update(boost::lexical_cast<int>(argv[2]),0);
    BasicTransaction* bTrans = BasicTransaction::factory(argv[1],r);
    bTrans->registerReader(rdr);
    bTrans->start();
    while (!bTrans->complete());
    return 0;

} catch (std::exception& ex) {
    std::cout<<ex.what()<<std::endl;
}

    /*RemoteData* rd = RemoteData::factory(argv[1]);
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
    return 0;*/

