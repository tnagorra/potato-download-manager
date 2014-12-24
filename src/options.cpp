#include "options/CommonOptions.h"
#include "options/GlobalOptions.h"
#include "options/LocalOptions.h"
#include "common/helper.h"
#include <vector>
#include <iostream>
#include <string>

const std::string globalConfig = "potas/global.ini";
const std::string localConfig= "local.ini";

int main(int ac, char* av[]) try {
    // Get Global Options
    GlobalOptions g;
    g.store(globalConfig);
    g.load();

    //
    // if nothing is specified then show things
    if( ac<=1 ){
        // Find the purgatory and look for sessions
        Directory session(g.destination_purgatory());
        if( !session.exists() || session.isEmpty()){
            std::cout << "No previous sessions!" << std::endl;
            return 0;
        }

        std::cout << "Previous sessions in " << g.destination_purgatory() << " :" << std::endl;

        // If session exists then print all the valid sessions
        std::vector<std::string> hash = session.list(Node::DIRECTORY);
        for(int i=0;i< hash.size();i++){
            std::string confname = hash[i]+"/" +localConfig;
            if(!File(confname).exists())
                continue;
            LocalOptions l;
            l.store(confname);
            l.load();

            // TODO Do some good stuff here
            std::cout << l.transaction_path() << std::endl;
        }

    } else {
        LocalOptions l;
        l.store(ac,av);

        // If it contains -h then show help immediately
        if(l.help()){
            std::cout << l.content() << std::endl;
            return 0;
        }

        // Only load global settings
        // Local settings can't be loaded in first run
        l.store(globalConfig);
        l.load();

        l.unload(g.destination_purgatory()+"/"+md5(l.transaction_path())+"/"+localConfig);

        // TODO Do some good stuff here
        std::cout << l.transaction_path() << std::endl;
    }

    return 0;
} catch (std::exception& ex){
    std::cout << ex.what() << std::endl;
    std::cout << "Try \'aggregator -h\' for more information." << std::endl;
}
