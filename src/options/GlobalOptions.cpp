#include "options/GlobalOptions.h"

GlobalOptions::GlobalOptions(){
    m_desc.add_options()
        ("destination.path,p",po::value< std::string >()->required(),
         "the destination path of the file")
        ("destination.purgatory,P",po::value< std::string >()->required(),
         "the temporary destination path of the file")
        ;
}
