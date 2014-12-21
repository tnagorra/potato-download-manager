#include "options/LocalOptions.h"

LocalOptions::LocalOptions(){
    m_desc.add_options()
        ("transaction.path",po::value< std::string >()->required(),
         "the path of the url")
        ;
    m_pos.add("transaction.path", -1);
}
