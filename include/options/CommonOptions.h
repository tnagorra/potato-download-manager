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
#include<fstream>
#include<string>
#include<vector>

#include "filesystem/File.h"

namespace po = boost::program_options;
namespace pt = boost::property_tree;

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

        // Notifes and checks everything
        void load();

        // Saves the variables map to a config file
        void unload(const std::string& filename);

        // Displays the values of options description
        void help();

        // Displays the values of variables map
        void display();
};

#endif

