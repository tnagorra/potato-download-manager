#include <boost/program_options/options_description.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/cmdline.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <iostream>
#include <aggregator/Aggregate.h>
#include <options/CommonOptions.h>
#include <options/LocalOptions.h>
#include <options/GlobalOptions.h>


const std::string globalConfig = "potas/global.ini";
const std::string localConfig= "local.ini";

int main(int ac, char* av[]) try {
    // Get Global Options
    GlobalOptions g;
    g.store(globalConfig);
    g.load();

    //
    // if nothing is specified then show things
    if ( ac<=1 ) {
        // Find the purgatory and look for sessions
        Directory session(g.destination_purgatory());
        if( !session.exists() || session.isEmpty()){
            std::cout << "No download history!" << std::endl;
            return 0;
        }

        std::cout << g.destination_purgatory() << "/" << std::endl;

        // If session exists then print all the valid sessions
        std::vector<std::string> hash = session.list(Node::DIRECTORY);
        int c=1;
        for(int i=0;i< hash.size();i++){
            std::string confname = hash[i]+"/" +localConfig;
            if(!File(confname).exists())
                continue;
            LocalOptions l;
            l.store(confname);
            l.load();

            // TODO Do some good stuff here
            std::cout << c++ << ". " << prettify(l.transaction_path()) << std::endl;
            std::cout << l.transaction_path() << std::endl;
        }

    } else {

        LocalOptions l;
        try {
            l.store(ac,av);
            // If it contains -h then show help immediately
            if(l.help()){
                std::cout << l.content() << std::endl;
                return 0;
            }
            // Load global config
            l.store(globalConfig);
            l.load();
        } catch ( std::exception& ex){
            print( ex.what() );
            fancyprint("Try \'aggregator -h\' for more information.",WARNING);
            return 0;
        }
        // Save local config
        l.unload(g.destination_purgatory()+"/"+md5(l.transaction_path())+"/"+localConfig);

        Aggregate agg(l.transaction_path(),g.destination_path(),
                g.destination_purgatory(),l.segment_number(),
                l.segment_threshold());
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

        fancyprint("Why am I still here?",NOTIFY);
    }
    return 0;
} catch (std::exception& ex){
    std::cout << ex.what() << std::endl;
}
