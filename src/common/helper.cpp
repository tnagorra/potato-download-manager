#include"common/helper.h"

std::string formatTime(uintmax_t sec) {
    if(sec == UINT_MAX)
        return "DAFUQ";
    std::string s;
    s = std::to_string(sec%60) + "s";
    sec /= 60;
    if(sec >0){
        s = std::to_string(sec%60) + "m"+s;
        sec /= 60;
        if( sec > 0){
            s = std::to_string(sec%24) + "h"+s;
            sec /= 24;
            if( sec > 0){
                s = std::to_string(sec%365) + "d"+s;
                sec /= 365;
                if(sec > 0){
                    s = std::to_string(sec)+ "y"+s;
                }
            }
        }
    }
    return s;
}

std::string formatByte(uintmax_t byte){
    uintmax_t c = 0;
    uintmax_t sec = byte;
    uintmax_t secprev = 0;
    while(sec/1024 >= 1){        // Firstbit must be atleast 1
        secprev = sec;
        sec /= 1024;
        c++;
    }
    if(secprev==0)
        secprev = sec*1024;
    std::ostringstream out;
    out << std::setprecision(2) << std::fixed << round(secprev/1024.0,2);
    switch(c){
        case 0: out << "B";break;
        case 1: out << "KiB";break;
        case 2: out << "MiB";break;
        case 3: out << "GiB";break;
        case 4: out << "TiB";break;
        case 5: out << "PiB";break;
        case 6: out << "EiB";break;
        case 7: out << "ZiB";break;
        case 8: out << "YiB";break;
        default : out << "DAFUQ";
    }
    return out.str();
}

std::string randomString(uintmax_t len) {
    // Initialize random seed
    srand (time(NULL));
    static const char alphanum[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789";
    static const unsigned max = sizeof(alphanum)/sizeof(*alphanum) - 1;

    std::string s;
    for (int i = 0; i < len; ++i)
        s += alphanum[rand() % max];
    return s;
}