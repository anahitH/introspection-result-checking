#include "SensitiveFunctionMarkPass.h"

#include "Utils.h"

#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/CommandLine.h"

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

#include <fstream>

namespace result_checking {

void SensitiveFunctionInformation::collect_sensitive_functions(const std::string& file_name, llvm::Module& M)
{
    std::ifstream functions_strm(file_name);
    std::string name;
    std::unordered_set<std::string> sensitive_function_names;
    while (!functions_strm.eof()) {
        functions_strm >> name;
        sensitive_function_names.insert(name);
    }

    for (auto& F : M) {
        auto demangled_name = NameUtilities::demangle(F.getName());
        if (demangled_name.empty()) {
            demangled_name = F.getName();
        }
        NameUtilities::extract_function_name(demangled_name);
        if (sensitive_function_names.find(demangled_name) != sensitive_function_names.end()) {
            llvm::dbgs() << "Add sensitive function " << demangled_name << "\n";
            add_sensitive_function(&F);
        }
    }
}

void SensitiveFunctionInformation::add_sensitive_function(llvm::Function* F)
{
    m_sensitive_functions.insert(F);
}

bool SensitiveFunctionInformation::is_sensitive_function(llvm::Function* F) const
{
    return m_sensitive_functions.find(F) != m_sensitive_functions.end();
}

const SensitiveFunctionInformation::FunctionSet& SensitiveFunctionInformation::get_sensitive_functions() const
{
    return m_sensitive_functions;
}

static llvm::cl::opt<std::string> InputFilename("sensitive-functions",
                                                llvm::cl::desc("Specify input filename for Sensitive function mark pass"),
                                                llvm::cl::value_desc("filename"));

char SensitiveFunctionMarkPass::ID = 0;

bool SensitiveFunctionMarkPass::runOnModule(llvm::Module& M)
{
    m_sensitive_functions_info.collect_sensitive_functions(InputFilename, M);

    return false;
}

static llvm::RegisterPass<SensitiveFunctionMarkPass> X("mark-functions","Marks functions in a given file as sensitive functions");
} // namespace result_checking


