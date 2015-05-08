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


int main(int ac, char * av[]) try {
    // Get Global Options
    GlobalOptions g;
    g.store(globalConfig);
    g.load();

    // if nothing is specified then display s
    if ( ac <= 1 ) {
        // Find the purgatory and look for sessions
        bool empty = true;

        // If active session exists then get all the sessions
        std::vector<std::string> active_sessions = Directory(g.destination_purgatory()).list(Node::DIRECTORY);
        for (int i = 0; i < active_sessions.size(); i++) {
            std::string confname = active_sessions[i] + "/" + localConfig;
            if (!File(confname).exists())
                continue;

            LocalOptions l;
            l.store(confname);
            l.load();
            show(l.transaction_path());
            try {
                Aggregate agg(l.transaction_path(), g.destination_path(),
                              g.destination_purgatory(), l.segment_number(),
                              l.segment_threshold());
                show(progressbar(agg.progress(), BARONE, BARTWO));
            } catch (ex::filesystem::NotThere& e) {
                // These are files that aren't resumable
                show(progressbar(0.00, BARONE, BARTHREE));
            }
            std::cout << std::endl;
            empty = false;
        }

        // If complete session exists then get all the sessions
        std::vector<std::string> complete_sessions = Directory(g.destination_path()).list(Node::FILE);
        for (int i = 0; i < complete_sessions.size(); i++) {
            show(complete_sessions[i]);
            show(progressbar(100.00, BARFOUR, BARONE));

            std::cout << std::endl;
            empty = false;
        }

    if (empty)
        fancyshow("No download history.", WARNING);

    } else {

        LocalOptions l;
        try {
            // Load immediate config
            l.store(ac, av);
            // Load global config
            l.store(globalConfig);


            // If it contains -h then show help immediately
            if (l.help()) {
                show(l.content());
                return 0;
            }
            // Notify
            l.load();
        } catch ( std::exception & ex) {
            fancyshow(ex.what(), ERROR);
            fancyshow("Try \'aggregator -h\' for more information.", WARNING);
            return 0;
        }

        Aggregate agg(l.transaction_path(), g.destination_path(),
                      g.destination_purgatory(), l.segment_number(),
                      l.segment_threshold());

        // Load Local config or else some data may disappear
        l.store(agg.purgatoryFilename()+"/"+localConfig);
        l.load();
        // Save local config
        l.unload(agg.purgatoryFilename() + "/" + localConfig);

        agg.start();
        while (!agg.isComplete() && !agg.hasFailed()) {
            unsigned count = agg.displayChunks();
            boost::this_thread::sleep(boost::posix_time::millisec(100));
            for (int i = 0; i < count; i++)
                std::cout << DELETE;
        }
        agg.join();

        agg.displayExceptions();

        // Sometimes waits here for a while.
        fancyshow("Closing!", NOTIFY);
    }
    return 0;
} catch (std::exception & ex) {
    fancyshow(ex.what(),ERROR);
}
