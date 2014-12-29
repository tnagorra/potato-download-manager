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
            fancyprint("No download history.",WARNING);
            return 0;
        }

        // If session exists then print all the valid sessions
        std::vector<std::string> hash = session.list(Node::DIRECTORY);

        bool empty = true;
        for(int i=0;i< hash.size();i++){
            std::string confname = hash[i]+"/" +localConfig;
            if(!File(confname).exists())
                continue;
            empty = false;
            LocalOptions l;
            l.store(confname);
            l.load();

            print( g.destination_path() << "/" << prettify(l.transaction_path()) );

            Aggregate agg(l.transaction_path(),g.destination_path(),
                    g.destination_purgatory(),l.segment_number(),
                    l.segment_threshold());

            std::cout << progressbar(agg.progress(),COLOR(0,CC::WHITE,CC::BLUE),COLOR(0,CC::BLUE,CC::WHITE));
            print( " " << round(agg.progress(),2) << "%\t");

            //agg.progressbar();
        }
        if(empty){
            fancyprint("No download history.",WARNING);
            return 0;
        }

    } else {

        LocalOptions l;
        try {
            l.store(ac,av);
            // If it contains -h then show help immediately
            if(l.help()){
                print(l.content());
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

        while (!agg.isComplete() && !agg.hasFailed()){

            unsigned all = agg.displayChunks();
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
