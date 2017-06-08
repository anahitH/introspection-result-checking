#pragma once

#include "llvm/Pass.h"

namespace result_checking {

class GuardsInserterPass : public llvm::ModulePass
{
public:
    static char ID;

    GuardsInserterPass()
        : llvm::ModulePass(ID)
    {
    }

public:
    void getAnalysisUsage(llvm::AnalysisUsage& AU) const override;
    bool runOnModule(llvm::Module& M) override;
};

}

