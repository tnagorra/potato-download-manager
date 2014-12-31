#include<iostream>
#include<string>
#include"filesystem/Directory.h"
#include"filesystem/File.h"
#include"common/helper.h"

int main() try {
    File nf("potatoes/programming-with-gtkmm.pdf");
    File tf("potatoes/programming-with-gtkmm.pdf");
    File gf("newfile.pdf");
    gf.write();
    //gf.append(nf,100,100);
    gf.append(nf,1000,0);
    gf.append(tf,0,1000+1);
    return 0;
    File f("hari");
    f.write(Node::NEW);
    //std::cout << prettify("https://r5---sn-npo7zn7z.googlevideo.com/videoplayback?fexp=904723%2C907259%2C916602%2C916644%2C927622%2C932404%2C943917%2C947209%2C948124%2C952302%2C952605%2C952901%2C953912%2C955006%2C957103%2C957105%2C957201%2C959801%2C959802&source=youtube&itag=22&gcr=np&ratebypass=yes&id=o-AMU2wLpAP5fba4xUh4mCsrXG7bSx9xhPL4cKEDp4eQcq&requiressl=yes&sver=3&ip=182.93.71.114&sparams=cwbf,dur,expire,gcr,id,initcwndbps,ip,ipbits,itag,mime,mm,ms,mv,ratebypass,requiressl,source,upn&mime=video%2Fmp4&key=cms1&cwbf=0.25&signature=758ADD42BE1059382CFBE3401E5074DF8133999D.1CB5E2101D221803B1FFE52CC2B002EDFAA1E272&upn=jKM_ajYgqyM&ipbits=0&expire=1417908017&dur=126.107&redirect_counter=1&req_id=1c2161c91d1aa3ee&cms_redirect=yes&mm=30&ms=nxu&mt=1417887880&mv=m") <<std::endl;
    return 0;
    File b(".temporary");
    b.write("My name is\b",Node::FORCE);
    b.append(" hari.\b");
    b.append("\nI live\b in Nepal.");
    if (b.exists()){
        b.move("ok",Node::FORCE);
        std::cout << b.size() << std::endl;
        b.remove();
    }
    std::cout << md5("hari") << std::endl;

    /*
    // Some Helper functions
    std::cout << formatTime(23823992)<<std::endl;
    std::cout << formatByte(1024*1024+343234)<<std::endl;
    std::cout << round(1243.234222343,3) << std::endl;
    std::cout << randomString()<<std::endl;
    std::cout << decodeUrl("Why%20the%20f**ck%20does%20%26%20not%20work")<<std::endl;
    */

 }
 catch (fs::filesystem_error& e){
    std::cout << e.what() << std::endl;
} catch (std::exception& e){
    std::cout << e.what() <<std::endl;
}
