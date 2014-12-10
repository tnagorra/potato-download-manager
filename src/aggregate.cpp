#include <iostream>
#include <aggregate/Aggregate.h>

//"http://onlinekaraoke.tv/assets/songs/29000-29999/29660-i-will-always-love-you-whitney-houston--1411573135.mp3"

int main(int argc, char* argv[]) try {
    if( argc == 2){
        Aggregate agg(argv[1]);
        agg.start();
        File f("attrib");
        f.remove();
        while( agg.totalChunks() == 0)
            boost::this_thread::sleep(boost::posix_time::millisec(100));
        while( !agg.complete() ){
           // std::cout << formatByte(agg.bytesTotal()) <<std::endl;

            agg.display();
            int all = agg.totalChunks();

            int len = 50;
            int place = agg.progress()/100*len;
            std::cout << COLOR(0,CC::WHITE,CC::PURPLE);
            for(int i=0;i< place;i++)
                std::cout << " ";
            std::cout << COLOR(0,CC::PURPLE,CC::WHITE);
            for(int i=place;i<len;i++)
                std::cout << " ";
            std::cout << DISCOLOR;

            std::cout << " " << round(agg.progress(),2) << "%\t";
            std::cout << formatTime(agg.timeRemaining()) << "\t";
            std::cout <<  formatByte(agg.speed()) << "ps\t";
            std::cout << std::endl;

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
