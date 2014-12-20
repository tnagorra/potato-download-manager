#include <boost/program_options/options_description.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/cmdline.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <iostream>
#include <aggregator/Aggregate.h>

namespace pt = boost::property_tree;
namespace po = boost::program_options;

int main(int argc, char* argv[]) try {
    if( argc == 2){
        Aggregate agg(argv[1]);
        agg.start();
        while( agg.totalChunks() == 0)
            boost::this_thread::sleep(boost::posix_time::millisec(100));
        while( !agg.isComplete() && !agg.hasFailed()){
            unsigned all = agg.display();
            agg.progressbar();
            boost::this_thread::sleep(boost::posix_time::millisec(100));
            for(int i=0;i<all+1;i++)
                std::cout<< DELETE;
        }
        agg.join();
    } else {
        fancyprint("Usage: aggregate [PATH]",WARNING);
    }
    return 0;
} catch (std::exception& ex) {
    fancyprint(ex.what(),ERROR);
}
