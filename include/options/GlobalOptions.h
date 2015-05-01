#ifndef __CO_GLOBALOPTIONS__
#define __CO_GLOBALOPTIONS__

#include "options/CommonOptions.h"

class GlobalOptions : public CommonOptions {
    public:
        GlobalOptions();

        std::string destination_path() const {
            if (m_vm.count("destination.path"))
                return m_vm["destination.path"].as<std::string>();
            return "potatoes";
        }

        std::string destination_purgatory() const {
            if (m_vm.count("destination.purgatory"))
                return m_vm["destination.purgatory"].as<std::string>();
            return "incomplet";
        }

};

#endif
