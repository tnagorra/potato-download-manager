#include <iostream>
#include <aggregate/Aggregate.h>

int main() {
    Aggregate agg("http://onlinekaraoke.tv/assets/songs/29000-29999/29660-i-will-always-love-you-whitney-houston--1411573135.mp3");
    //Aggregate agg("localhost/ubuntu.iso");
    agg.start();
    agg.join();
    std::cout<<"shit!"<<std::endl;
    return 0;
}
