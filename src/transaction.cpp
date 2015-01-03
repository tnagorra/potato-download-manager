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

#include "common/ExBridge.h"

int main(int argc, char* argv[]) try {
    Range r;
    std::string filename="potato";
    if (argc>2)
        filename = argv[2];
    if (argc>3)
        r.update(boost::lexical_cast<uintmax_t>(argv[3]),0);
    ExBridge* eb = new ExBridge;
    BasicTransaction* bTrans = BasicTransaction::factory(argv[1],r);
    bTrans->exbridge(eb);

    File potato(filename,File::BINARY);
    boost::function<void (std::istream&, uintmax_t,uintmax_t)> rdr;
    rdr = boost::bind(static_cast<void(File::*)(std::istream&,
                uintmax_t,uintmax_t)>(&File::append), &potato, _1, _2, _3);
    bTrans->registerReader(rdr);

    bTrans->start();
    bTrans->join();
    return 0;

} catch (std::exception& ex) {
    std::cout<<ex.what()<<std::endl;
}
