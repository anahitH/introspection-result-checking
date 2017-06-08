#include "GuardsInserter.h"

#include "GuardsNetwork.h"
#include "SensitiveFunctionMarkPass.h"

#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/Constants.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/CommandLine.h"

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"


namespace result_checking {

namespace {

/*
Non pure instrumented function will look like
    f()
    {
        call_checkers
        function body
    }

pure instrumented function will look like
this will solve cycle problem for sequential code
    f()
    {
        static guard_run = false;
        if (!guard_run) {
            guard_run = true;
            call_checkers;
            guard_run = false;
        }
        function_body
    }
*/
void insert_guards(GuardNetwork::GuardNode& guard_node, llvm::DominatorTree* dom, llvm::LoopInfo* loop_info, bool is_sensitive_function)
{
    llvm::Function* F = guard_node.get_function();
    llvm::dbgs() << "Inserting guards to function " << F->getName() << "\n";
    llvm::LLVMContext &Ctx = F->getContext();

    llvm::BasicBlock &bb = F->getEntryBlock();
    llvm::IRBuilder<> builder(&bb);
    builder.SetInsertPoint(&bb, ++builder.GetInsertPoint());


    // get or create checker function
    if (!is_sensitive_function) {
        //create call to sensitive function
        return;
        
    }
    // Instrument function to cope with ciclic calls
    llvm::dbgs() << "Inserting guards for sensitive function\n";

    const std::string f_name = F->getName();
    const std::string var_name = "checks_run." + f_name;
    llvm::GlobalVariable* guard_flag = new llvm::GlobalVariable(*F->getParent(),
                                                                llvm::Type::getInt1Ty(Ctx),
                                                                false,
                                                                llvm::GlobalValue::PrivateLinkage,
                                                                llvm::ConstantInt::getFalse(llvm::Type::getInt1Ty(Ctx)),
                                                                var_name.c_str());
    auto& first_inst = *bb.begin();
    // Check if guard flag is set
    auto load_guard_flag = builder.CreateLoad(guard_flag);
    auto trunc_guard_flag = builder.CreateTrunc(load_guard_flag, llvm::Type::getInt1Ty(Ctx));
    auto cmp_value = builder.CreateICmpEQ(trunc_guard_flag, llvm::ConstantInt::getFalse(llvm::Type::getInt1Ty(Ctx)));

    // split entry block of function into two parts, first is check above, second is normal body of the function
    auto split_block = llvm::SplitBlock(&bb, &first_inst, dom);
    builder.SetInsertPoint(&bb);

    // add block for checker function calls
    auto checker_block = llvm::BasicBlock::Create(Ctx, "if.then", F, split_block);
    // Apparently llvm does not understand that IBuilder created branch instruction is terminator.
    // Explicily create TerminatorInst and add to instruction list back
    // Note: after checker call, function body is going to be executed
    llvm::TerminatorInst* inst = llvm::BranchInst::Create(split_block);
    checker_block->getInstList().push_back(inst);

    // Modify entry (new) block terminator to redirect to either checker calls or normal body
    llvm::TerminatorInst* entry_term = llvm::BranchInst::Create(checker_block, split_block, cmp_value);
    bb.getInstList().pop_back();
    bb.getInstList().push_back(entry_term);

    builder.SetInsertPoint(&checker_block->getInstList().back());
    builder.CreateStore(llvm::ConstantInt::getTrue(llvm::Type::getInt1Ty(Ctx)), guard_flag);
//    // call checkers
    builder.CreateStore(llvm::ConstantInt::getFalse(llvm::Type::getInt1Ty(Ctx)), guard_flag);

    llvm::dbgs() << "Inserted\n";
}

}

char GuardsInserterPass::ID = 0;

void GuardsInserterPass::getAnalysisUsage(llvm::AnalysisUsage& AU) const
{
    AU.addRequired<GuardNetworkCreatorPass>();
    //AU.addPreserved<GuardNetworkCreatorPass>();
    AU.addRequired<SensitiveFunctionMarkPass>();
    AU.addRequired<llvm::DominatorTreeWrapperPass>();
    AU.addRequired<llvm::LoopInfoWrapperPass>();
    //AU.addPreserved<SensitiveFunctionMarkPass>();
}

bool GuardsInserterPass::runOnModule(llvm::Module& M)
{
    auto& guards_network = getAnalysis<GuardNetworkCreatorPass>().get_guards_network();
    auto& sensitive_function_info = getAnalysis<SensitiveFunctionMarkPass>().get_sensitive_functions_info();
    if (guards_network.is_empty()) {
        return false;
    }
    for (auto& F : M) {
        auto dom_tree = &getAnalysis<llvm::DominatorTreeWrapperPass>(F).getDomTree();
        auto loop_info = &getAnalysis<llvm::LoopInfoWrapperPass>(F).getLoopInfo();
        if (F.isDeclaration()) {
            continue;
        }
        if (!guards_network.is_guard(&F)) {
            continue;
        }
        insert_guards(guards_network.get_guard_for(&F), dom_tree, loop_info, sensitive_function_info.is_sensitive_function(&F));
    }
    return true;
}

static llvm::RegisterPass<GuardsInserterPass> X("insert-guards","Insert guards in guard functions");
}

