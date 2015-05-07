#include"common/helper.h"

std::string formatTime(uintmax_t sec) {
    if(sec == std::numeric_limits<uintmax_t>::max())
        return "dafuq!";
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
        default : out << "dafuq!";
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

std::string md5(std::string text) {
    // Hashing
    unsigned char digest[MD5_DIGEST_LENGTH];
    MD5_CTX ctx;
    MD5_Init(&ctx);
    MD5_Update(&ctx, text.c_str(), text.length());
    MD5_Final(digest, &ctx);
    // Converting the integer into hex
    std::ostringstream out;
    out << std::hex << std::setfill('0');
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
        out << std::setw(2)  << (unsigned)digest[i];
    return out.str();
}

std::string prettify(std::string url){
    // Finds the name by getting the name after
    // the last "/" and the before the first "?"
    // after that
    size_t last_slash = url.find_last_of('/')+1;
    size_t first_ques= url.find('?',last_slash)-1;
    if( first_ques == std::string::npos)
        first_ques = url.length();
    return decodeUrl(url.substr(last_slash,first_ques-last_slash+1));
}

// Remove the default port 80 from url
std::string removeport80(std::string url) {
    size_t colon = url.find(':');
    size_t slash = url.find('/',colon);
    if (colon==std::string::npos)
        return url;
    if (slash==std::string::npos)
        slash = url.size();
    if (url.substr(colon+1,slash-colon-1)!="80") {
        return url;
    }
    return url.substr(0,colon)+url.substr(slash,url.size()-slash);
}

int charToHex(char x){
    if (x>='a')
        x -= 'a'-'A';
    if (x >= 'A')
        x -= 'A'-10;
    else
        x -= '0';
    return x;
}

std::string decodeUrl(std::string url){
    std::string lru;
    char a,b;
    unsigned c = 0;
    for(c=0;c < url.length();c++){
        if( url[c]=='%' && ((a=url[c+1]) && (b=url[c+2])) && (std::isxdigit(a) && std::isxdigit(b)) ) {
            lru +=16*charToHex(a)+charToHex(b);
            c+=2;   // Two extra char are read
        } else {
            lru += url[c];
        }
    }
    return lru;
}

bool numerically(std::string const & a, std::string const & b){
    return stoi(a) < stoi(b);
}
bool isNumeric(const std::string& s) {
    auto it = s.begin();
    while (it != s.end() && std::isdigit(*it))
        ++it;
    return !s.empty() && it == s.end();
}

std::string progressbar(float progress,std::string yes, std::string no,unsigned len) {
    std::stringstream stream;
    unsigned place = progress/100*len;
    if(len-place > 0)
        stream << std::fixed << std::setprecision(2)
            << yes << std::string(place,' ')
            << no << std::string(len-place,' ')
            << DISCOLOR << " " << round(progress,2) << "%\t";
    return  stream.str();
}
