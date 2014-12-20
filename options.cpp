#include<boost/property_tree/ptree.hpp>
#include<boost/property_tree/ini_parser.hpp>
#include<boost/program_options/options_description.hpp>
#include<boost/program_options/positional_options.hpp>
#include<boost/program_options/variables_map.hpp>
#include<boost/program_options/cmdline.hpp>
#include<boost/program_options/parsers.hpp>
#include<iostream>
#include<fstream>
#include<string>
#include<vector>

namespace po = boost::program_options;
namespace pt = boost::property_tree;

class CommonOptions {
    protected:
        po::options_description m_desc;
        po::positional_options_description m_pos;
        po::variables_map m_vm;
    public:
        CommonOptions(): m_desc("Options") {
            m_desc.add_options()
                ("help,h", "produce help message")
                ("segment.threshold",po::value<uintmax_t>()->default_value(1024*100),
                 "the smallest size of a segment")
                ("segment.number",po::value<unsigned>()->default_value(8),
                 "the total no. of active segments")
                ("transaction.timeout",po::value<unsigned>()->default_value(80),
                 "the timeout for the transaction")
                ("transaction.retries",po::value<int>()->default_value(-1),
                 "the total no. of retries for transaction")
                ("transaction.wait",po::value<unsigned>()->default_value(10),
                 "the wait for the transaction")
                ("file.mode",po::value<std::string>()->default_value(std::string("rename")),
                 "the mode for file conflict")
                ;
        }

        void store(int ac, char* av[]){
            if(m_pos.name_for_position(0)=="")
                po::store(po::command_line_parser(ac, av).options(m_desc).run(), m_vm);
            else
                po::store(po::command_line_parser(ac, av).options(m_desc).positional(m_pos).run(), m_vm);
        }

        void store(const std::string& filename){
            std::fstream m_stream;
            m_stream.open(filename,std::ios::in);
            po::store(po::parse_config_file(m_stream, m_desc, true), m_vm);
        }

        void load() {
            po::notify(m_vm);
        }

        void display() {

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

        void save(const std::string& filename){
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

            std::fstream conffile;
            conffile.open(filename,std::ios::out);
             write_ini(conffile,root);
        }
};

class GlobalOptions : public CommonOptions {
    public:
        GlobalOptions(){
            m_desc.add_options()
                ("destination.path",po::value< std::string >()->required(),
                 "the destination path of the file")
                ("destination.purgatory",po::value< std::string >()->required(),
                 "the temporary destination path of the file")
                ;
        }
};

class LocalOptions : public CommonOptions {
    public:
        LocalOptions(){
            m_desc.add_options()
                ("transaction.path",po::value< std::string >()->required(),
                 "the path of the url")
                ;
            m_pos.add("transaction.path", -1);
        }
};

int main(int ac, char* av[]) try {
    LocalOptions l;
    l.store(ac,av);
    l.store("potatoes/local.ini");
    l.store("potatoes/global.ini");
    l.load();
    l.display();
    l.save("potatoes/merger.ini");
    return 0;
} catch (std::exception& ex){
    std::cout << ex.what() << std::endl;
}
