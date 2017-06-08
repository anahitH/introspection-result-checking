
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

#include "SensitiveFunctionMarkPass.h"
#include "FunctionDominanceTree.h"
#include "GuardsNetwork.h"
#include "GuardsInserter.h"

namespace result_checking {

static void registerPathsAnalysisPass(const llvm::PassManagerBuilder &,
                         	      llvm::legacy::PassManagerBase &PM) {
  //PM.add(new CallPathsAnalysisPass());
  PM.add(new SensitiveFunctionMarkPass());
  PM.add(new FunctionDominanceTreePass());
  PM.add(new GuardNetworkCreatorPass());
  PM.add(new GuardNetworkCreatorPrinterPass());
  PM.add(new GuardsInserterPass());
}

static llvm::RegisterStandardPasses RegisterMyPass(llvm::PassManagerBuilder::EP_EarlyAsPossible, registerPathsAnalysisPass);

}

