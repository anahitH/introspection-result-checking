#include "llvm/Pass.h"

#include <unordered_set>

namespace llvm {
class Function;
}

namespace result_checking {

class SensitiveFunctionMarkPass : public llvm::ModulePass
{
public:
    class SensitiveFunctionInformation
    {
    public:
        SensitiveFunctionInformation() = default;
        SensitiveFunctionInformation(const SensitiveFunctionInformation&) = delete;
        SensitiveFunctionInformation& operator =(const SensitiveFunctionInformation&) = delete;

    public:
        void add_sensitive_function(llvm::Function* F);

        bool is_sensitive_function(llvm::Function* F) const;

    private:
        std::unordered_set<llvm::Function*> m_sensitive_functions;
    }; // class SensitiveFunctionInformation

public:
    static char ID;

public:
    SensitiveFunctionMarkPass()
        : llvm::ModulePass(ID)
    {
    }

public:
    bool runOnModule(llvm::Module& M) override;

    const SensitiveFunctionInformation& get_sensitive_functions_inf() const
    {
        return m_sensitive_functions_info;
    }

private:
    SensitiveFunctionInformation m_sensitive_functions_info;
}; // class SensitiveFunctionMarkPass

} // namespace result_checking
