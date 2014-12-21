#include "options/CommonOptions.h"

CommonOptions::CommonOptions(): m_desc("Options") {
    m_desc.add_options()
        ("help,h", "produce help message")
        ("segment.threshold,T",po::value<uintmax_t>()->default_value(1024*100),
         "the smallest size of a segment")
        ("segment.number,N",po::value<unsigned>()->default_value(8),
         "the total no. of active segments")
        ("transaction.timeout,t",po::value<unsigned>()->default_value(80),
         "the timeout for the transaction")
        ("transaction.retries,r",po::value<int>()->default_value(-1),
         "the total no. of retries for transaction")
        ("transaction.wait,w",po::value<unsigned>()->default_value(10),
         "the wait for the transaction")
        ("file.mode,m",po::value<std::string>()->default_value(std::string("rename")),
         "the mode for file conflict")
        ;
}

void CommonOptions::store(int ac, char* av[]){
    if(m_pos.name_for_position(0)=="")
        po::store(po::command_line_parser(ac, av).options(m_desc).run(), m_vm);
    else
        po::store(po::command_line_parser(ac, av).options(m_desc).positional(m_pos).run(), m_vm);
}

void CommonOptions::store(const std::string& filename){
    File conf(filename);
    if( conf.exists())
        po::store(po::parse_config_file(conf.istream(), m_desc, true), m_vm);
}

void CommonOptions::load() {
    po::notify(m_vm);
}

void CommonOptions::unload(const std::string& filename){
    pt::ptree root;
    pt::ptree branch = pt::ptree();
    std::string old;
    for (const auto& it : m_vm) {
        std::string fullname = it.first;
        size_t pos = fullname.find('.');
        std::string group = fullname.substr(0,pos);
        std::string name = fullname.substr(pos+1,fullname.length()-pos);

        if( old!=group){
            if(old!=""){
                root.push_back(pt::ptree::value_type(old, branch));
                branch = pt::ptree();
            }
            old = group;
        }

        auto& value = it.second.value();
        if (auto v = boost::any_cast<int>(&value))
            branch.put(name,std::to_string(*v));
        else if (auto v = boost::any_cast<unsigned>(&value))
            branch.put(name,std::to_string(*v));
        else if (auto v = boost::any_cast<uintmax_t>(&value))
            branch.put(name,std::to_string(*v));
        else if (auto v = boost::any_cast<std::string>(&value))
            branch.put(name,*v);
        else if (auto v = boost::any_cast< std::vector< std::string > >(&value)){
            std::string s;
            for(auto it=v->begin(); it!=v->end();++it)
                s += *it + " ";
            branch.put(name,s);

        } else
            branch.put(name,"error");
    }
    root.push_back(pt::ptree::value_type(old, branch));

    File conf(filename);
    write_ini(conf.ostream(),root);
}

void CommonOptions::help(){
    std::cout << m_desc << std::endl;
}

void CommonOptions::display() {

    for (const auto& it : m_vm) {
        std::cout << it.first.c_str() << "=";
        auto& value = it.second.value();
        if (auto v = boost::any_cast<int>(&value))
            std::cout << *v << std::endl;
        else if (auto v = boost::any_cast<unsigned>(&value))
            std::cout << *v << std::endl;
        else if (auto v = boost::any_cast<uintmax_t>(&value))
            std::cout << *v << std::endl;
        else if (auto v = boost::any_cast<std::string>(&value))
            std::cout << *v <<std::endl;
        else if (auto v = boost::any_cast< std::vector< std::string > >(&value)){
            for(auto it=v->begin(); it!=v->end();++it)
                std::cout << *it << " ";
            std::cout <<  std::endl;
        }
        else
            std::cout << "error" <<std::endl;
    }
}

