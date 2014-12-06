#include <iostream>
#include <aggregate/Aggregate.h>

//"http://onlinekaraoke.tv/assets/songs/29000-29999/29660-i-will-always-love-you-whitney-houston--1411573135.mp3"

#define COLOR "\033[0;36;40m"
#define PROGRESSY "\033[0;37;45m"
#define PROGRESSN "\033[0;37;47m"
#define DISCOLOR "\033[0m"
#define DELETE "\033[A\033[2K"



int main(int argc, char* argv[]) try {
    if( argc == 2){
        Aggregate agg(argv[1]);
        agg.start();
        while( agg.totalChunks() == 0)
            boost::this_thread::sleep(boost::posix_time::millisec(100));
        while( !agg.complete() ){
            std::cout << formatByte(agg.bytesTotal()) <<std::endl;
            std::cout << agg.activeChunks() << "/" << agg.totalChunks() <<"\t";
            std::cout <<  formatByte(agg.speed()) << "ps\t";
            std::cout << formatTime(agg.timeRemaining()) << "\t";
            std::cout << std::endl;

            int len = 50;
            int place = agg.progress()/100*len;
            std::cout << PROGRESSY;
            for(int i=0;i< place;i++)
                std::cout << " ";
            std::cout << PROGRESSN;
            for(int i=place;i<len;i++)
                std::cout << " ";
            std::cout << DISCOLOR;
            std::cout << " " << round(agg.progress(),2) << "%";
            std::cout << std::endl;

            boost::this_thread::sleep(boost::posix_time::millisec(100));
            std::cout<< DELETE DELETE DELETE;
        }
        agg.join();
    } else {
        std::cout << "Usage: aggregate [PATH]\n" << std::endl;
    }
    return 0;
} catch (std::exception& ex) {
    std::cout<<ex.what()<<std::endl;
}
