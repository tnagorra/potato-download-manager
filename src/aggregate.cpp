#include <iostream>
#include <aggregate/Aggregate.h>

//"http://onlinekaraoke.tv/assets/songs/29000-29999/29660-i-will-always-love-you-whitney-houston--1411573135.mp3"

int main(int argc, char* argv[]) try {
    if( argc == 2){
        Aggregate agg(argv[1]);
        agg.start();
        agg.join();
    } else {
        std::cout << "Noob" <<std::endl;
    }
    return 0;
} catch (std::exception& ex) {
    std::cout<<ex.what()<<std::endl;
}
