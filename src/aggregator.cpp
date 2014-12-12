#include <iostream>
#include <aggregator/Aggregate.h>

// TODO
// 1. Use a Range object reference as argument
// 2. Rename p_functions() to functions()
// 3. change state()==something to joinable()

int main(int argc, char* argv[]) try {
    if( argc == 2){
        Aggregate agg(argv[1]);
        agg.start();
        while( agg.totalChunks() == 0)
            boost::this_thread::sleep(boost::posix_time::millisec(100));
        while( !agg.isComplete() ){

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
