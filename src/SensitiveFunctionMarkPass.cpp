#include "SensitiveFunctionMarkPass.h"

#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/CommandLine.h"

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

#include <cxxabi.h>
#include <fstream>

namespace result_checking {

namespace {

std::string demangle(const std::string& mangled_name)
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

void extract_function_name(std::string& full_name)
{
    auto name_end = full_name.find_first_of("(");
    assert(name_end != std::string::npos);
    full_name = full_name.substr(0, name_end);
}

}

void SensitiveFunctionMarkPass::SensitiveFunctionInformation::add_sensitive_function(llvm::Function* F)
{
    m_sensitive_functions.insert(F);
}

bool SensitiveFunctionMarkPass::SensitiveFunctionInformation::is_sensitive_function(llvm::Function* F) const
{
    return m_sensitive_functions.find(F) != m_sensitive_functions.end();
}

static llvm::cl::opt<std::string> InputFilename("sensitive-functions",
                                                llvm::cl::desc("Specify input filename for Sensitive function mark pass"),
                                                llvm::cl::value_desc("filename"));

char SensitiveFunctionMarkPass::ID = 0;

bool SensitiveFunctionMarkPass::runOnModule(llvm::Module& M)
{
    const std::string& file_name = InputFilename;
    std::ifstream functions_strm(file_name);
    std::string name;
    std::unordered_set<std::string> sensitive_function_names;
    while (!functions_strm.eof()) {
        functions_strm >> name;
        sensitive_function_names.insert(name);
    }

    for (auto& F : M) {
        auto demangled_name = demangle(F.getName());
        if (demangled_name.empty()) {
            demangled_name = F.getName();
        }
        extract_function_name(demangled_name);
        if (sensitive_function_names.find(demangled_name) != sensitive_function_names.end()) {
            llvm::dbgs() << "Add sensitive function " << demangled_name << "\n";
            m_sensitive_functions_info.add_sensitive_function(&F);
        }
    }
    return false;
}

static llvm::RegisterPass<SensitiveFunctionMarkPass> X("mark-functions","Marks functions in a given file as sensitive functions");
} // namespace result_checking


