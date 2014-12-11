#include <iostream>
#include <aggregator/Aggregate.h>

//"http://onlinekaraoke.tv/assets/songs/29000-29999/29660-i-will-always-love-you-whitney-houston--1411573135.mp3"

int main(int argc, char* argv[]) try {
    if( argc == 2){
        Aggregate agg(argv[1]);
        agg.start();
        while( agg.totalChunks() == 0)
            boost::this_thread::sleep(boost::posix_time::millisec(100));
        while( !agg.isComplete() ){

            int all = agg.totalChunks();
            agg.display();
            agg.progressbar();
            boost::this_thread::sleep(boost::posix_time::millisec(100));
            for(int i=0;i<all+2;i++)
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
