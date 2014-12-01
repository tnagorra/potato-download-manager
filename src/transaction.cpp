#include <typeinfo>
#include <iostream>
#include <istream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include "common/ex.h"
#include "transaction/ex.h"
#include "transaction/Range.h"
#include "transaction/RemoteDataHttp.h"
#include "transaction/HttpTransaction.h"

#include "filesystem/File.h"
#include "filesystem/Directory.h"

void reader(std::istream& instream, uintmax_t n) {
    std::cout<<instream.rdbuf();
}

int main(int argc, char* argv[]) try {
    File potato("shyam/harisadu");
    boost::function<void (std::istream&, uintmax_t)> rdr;
    rdr = boost::bind(static_cast<void(File::*)(std::istream&,uintmax_t)>(&File::append), &potato, _1, _2);

    Range r;
    if (argc==3)
        r.update(boost::lexical_cast<uintmax_t>(argv[2]),0);
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

