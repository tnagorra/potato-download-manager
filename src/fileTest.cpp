#include<iostream>
#include<string>
#include"filesystem/Directory.h"
#include"filesystem/File.h"

int main() try {
    File b(".temporary");
    b.write("My name is\b",Node::FORCE);
    b.append(" hari.\b");
    b.append("\nI live\b in Nepal.");
    if (b.exists()){
        b.move("ok",Node::FORCE);
        std::cout << b.size() << std::endl;
    }
 }
 catch (fs::filesystem_error& e){
    std::cout << e.what() << std::endl;
} catch (std::exception& e){
    std::cout << e.what() <<std::endl;
}
