#pragma once

#include <cxxabi.h>

namespace result_checking {

class NameUtilities
{
public:
    static std::string demangle(const std::string& mangled_name)
    {
        int status = -1;
        char* demangled = abi::__cxa_demangle(mangled_name.c_str(), NULL, NULL, &status);
        if (status == 0) {
            return std::string(demangled);
        }
        //else {
        //    llvm::dbgs() << "Failed to demangle the name " << mangled_name << "\n";
        //}
        return std::string();
    }

    static void extract_function_name(std::string& full_name)
    {
        auto name_end = full_name.find_first_of("(");
        if (name_end != std::string::npos) {
            full_name = full_name.substr(0, name_end);
        }
    }
};

}

