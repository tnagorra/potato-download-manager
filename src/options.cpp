#include "options/CommonOptions.h"
#include "options/GlobalOptions.h"
#include "options/LocalOptions.h"

int main(int ac, char* av[]) try {
    LocalOptions l;
    l.store(ac,av);
    l.store("potatoes/local.ini");
    l.store("potatoes/global.ini");
    l.load();
//    l.help();
    l.display();
    l.unload("potatoes/merger.ini");
    return 0;
} catch (std::exception& ex){
    std::cout << ex.what() << std::endl;
}
