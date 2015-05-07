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

    // if nothing is specified then display sessions
    if ( ac <= 1 ) {
        // Find the purgatory and look for sessions
        bool empty = true;

        // If session exists then show all the valid sessions
        std::vector<std::string> sessionlist = Directory(g.destination_purgatory()).list(Node::DIRECTORY);

        for (int i = 0; i < sessionlist.size(); i++) {
            std::string confname = sessionlist[i] + "/" + localConfig;
            if (!File(confname).exists())
                continue;

            LocalOptions l;
            l.store(confname);
            l.load();
            show(l.transaction_filename());
            try {
                Aggregate agg(l.transaction_path(), g.destination_path(),
                              g.destination_purgatory(), l.segment_number(),
                              l.segment_threshold());
                show(progressbar(agg.progress(), BARONE, BARTWO));
            } catch (ex::filesystem::NotThere& e) {
                // These are files that aren't resumable
                show(progressbar(0.00, BARONE, BARTHREE));
            }
            empty = false;
        }

        std::vector<std::string> session = Directory(g.destination_path()).list(Node::FILE);
        for (int i = 0; i < session.size(); i++) {
            show(session[i]);
            show(progressbar(100.00, BARFOUR, BARONE));
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
            show( ex.what() );
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

        // TODO No idea why there program waits after this statement
        // Could be because of writing process or keep-alive connection.
        fancyshow("Just Wait.", NOTIFY);
    }
    return 0;
} catch (std::exception & ex) {
    fancyshow(ex.what(),ERROR);
}
