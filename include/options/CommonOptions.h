#ifndef __CO_COMMONOPTIONS__
#define __CO_COMMONOPTIONS__

#include<boost/property_tree/ptree.hpp>
#include<boost/property_tree/ini_parser.hpp>
#include<boost/program_options/options_description.hpp>
#include<boost/program_options/positional_options.hpp>
#include<boost/program_options/variables_map.hpp>
#include<boost/program_options/cmdline.hpp>
#include<boost/program_options/parsers.hpp>

#include<iostream>
#include<sstream>
#include<fstream>
#include<string>
#include<vector>

#include "filesystem/File.h"

namespace po = boost::program_options;
namespace pt = boost::property_tree;

const std::string globalConfig = "global.ini";
const std::string localConfig = "local.ini";

class CommonOptions {
    protected:
        po::options_description m_desc;
        po::positional_options_description m_pos;
        po::variables_map m_vm;
    public:
        CommonOptions();

        // Gets variables map from the commandline
        void store(int ac, char* av[]);

        // Gets variables map from the config file
        void store(const std::string& filename);

        // Notifies and checks everything
        void load();

        // Saves the variables map to a config file
        void unload(const std::string& filename);

        // Displays the values of variables map
        // For debugging
         void display();

        po::variables_map& variablesMap() {
            return m_vm;
        }

         bool help() const {
             if (m_vm.count("help"))
                 return true;
             return false;
         }

         std::string content() const {
             std::ostringstream os;
             os << m_desc;
             return os.str();
         }

         uintmax_t segment_threshold() const {
             if (m_vm.count("segment.threshold"))
                 return m_vm["segment.threshold"].as<uintmax_t>();
             return 1024*100;
         }

         unsigned segment_number() const {
             if (m_vm.count("segment.number"))
                 return m_vm["segment.number"].as<unsigned>();
             return 8;
         }

         unsigned transaction_timeout() const {
             if (m_vm.count("transaction.timeout"))
                 return m_vm["transaction.timeout"].as<unsigned>();
             return 80;
         }

         int transaction_retries() const {
             if (m_vm.count("transaction.retries"))
                 return m_vm["transaction.retries"].as<int>();
             return -1;
         }

         unsigned transaction_wait() const {
             if (m_vm.count("transaction.wait"))
                 return m_vm["transaction.wait"].as<unsigned>();
             return 10;
         }

         Node::Conflict file_conflict() const {
             if (m_vm.count("file.conflict")){
                 std::string c = m_vm["file.conflict"].as<std::string>();
                 if(c=="leave")
                     return Node::LEAVE;
                 else if(c=="force")
                     return Node::FORCE;
             }
             return Node::NEW;
         }
};

#endif

