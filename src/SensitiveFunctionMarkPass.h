#pragma once

#include "llvm/Pass.h"

#include <unordered_set>

namespace llvm {
class Function;
}

namespace result_checking {

class SensitiveFunctionInformation
{
public:
    using FunctionSet = std::unordered_set<llvm::Function*>;

public:
    SensitiveFunctionInformation() = default;
    SensitiveFunctionInformation(const SensitiveFunctionInformation&) = delete;
    SensitiveFunctionInformation& operator =(const SensitiveFunctionInformation&) = delete;

public:
    void collect_sensitive_functions(const std::string& file_name, llvm::Module& M);

    void add_sensitive_function(llvm::Function* F);
    bool is_sensitive_function(llvm::Function* F) const;
    const FunctionSet& get_sensitive_functions() const;

private:
    FunctionSet m_sensitive_functions;
}; // class SensitiveFunctionInformation

class SensitiveFunctionMarkPass : public llvm::ModulePass
{
public:
    static char ID;

public:
    SensitiveFunctionMarkPass()
        : llvm::ModulePass(ID)
    {
    }

public:
    bool runOnModule(llvm::Module& M) override;

    const SensitiveFunctionInformation& get_sensitive_functions_info() const
    {
        return m_sensitive_functions_info;
    }

private:
    SensitiveFunctionInformation m_sensitive_functions_info;
}; // class SensitiveFunctionMarkPass

} // namespace result_checking
