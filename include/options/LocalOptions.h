#ifndef __CO_LOCALOPTIONS__
#define __CO_LOCALOPTIONS__

#include "options/CommonOptions.h"

class LocalOptions : public CommonOptions {
    public:
        LocalOptions();

        std::string transaction_path() const {
            if (m_vm.count("transaction.path"))
                return m_vm["transaction.path"].as<std::string>();
            return "dafuq";
        }

};

#endif
