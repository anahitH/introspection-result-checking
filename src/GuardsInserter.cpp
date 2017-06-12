#include "GuardsInserter.h"

#include "GuardsNetwork.h"
#include "SensitiveFunctionMarkPass.h"
#include "FunctionsTestCaseManager.h"
#include "Utils.h"

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
  the checkee call looks like
  
  bool result = true;


  return_value = checkee(arg1, arg2, ..);
  result = result & (return_value == expected_return_value);

*/
void insert_call_for_testcase(llvm::Function* guard, llvm::Value* result, llvm::Function* checkee,
                              const FunctionTestCase::TestCaseType& test, llvm::IRBuilder<>& builder)
{
    assert(test.size() == checkee->getArgumentList().size() + 1);
    auto& Ctx = guard->getContext();

    std::vector<llvm::Value*> arg_values;
    llvm::Value* return_value = test[0]->to_llvm_value(Ctx);
    for (unsigned i = 1; i < test.size(); ++i) {
        arg_values.push_back(test[i]->to_llvm_value(Ctx));
    }
    llvm::ArrayRef<llvm::Value*> args(arg_values);
    auto call_value = builder.CreateCall(checkee, args);
    llvm::dbgs() << *call_value->getType() << "\n";
    llvm::dbgs() << *return_value->getType() << "\n";
    auto cmp_value = builder.CreateICmpEQ(call_value, return_value);

    auto load_result = builder.CreateLoad(result);
    auto trunc_result = builder.CreateTrunc(load_result, llvm::Type::getInt1Ty(Ctx));
    auto and_res = builder.CreateAnd(trunc_result, cmp_value);
    builder.CreateStore(and_res, result);
}

void insert_checkee_calls(GuardNetwork::GuardNode& guard_node, llvm::IRBuilder<>& builder)
{
    llvm::Function* guard = guard_node.get_function();
    auto& Ctx = guard->getContext();
    //llvm::Value* result = llvm::ConstantInt::get(llvm::Type::getInt8Ty(Ctx), 1);
    std::string var_name = "result"; 
    llvm::GlobalVariable* result = new llvm::GlobalVariable(*guard->getParent(),
                                                            llvm::Type::getInt1Ty(Ctx),
                                                            false,
                                                            llvm::GlobalValue::PrivateLinkage,
                                                            llvm::ConstantInt::getFalse(llvm::Type::getInt1Ty(Ctx)),
                                                            var_name.c_str());
 
    auto& test_case_manager = FunctionsTestCaseManager::get();
    for (const auto& checkee : guard_node.get_checkees()) {
        auto name = NameUtilities::demangle(checkee->getName());
        NameUtilities::extract_function_name(name);
        llvm::dbgs() << "Inserting check calls for checkee: " << name << "\n";
        const auto& test_cases = test_case_manager.get_function_test_case(name).get_test_cases();
        for (const auto& test : test_cases) {
            assert(test.size() == checkee->getArgumentList().size() + 1);
            std::vector<llvm::Value*> arg_values;
            llvm::Value* return_value = test[0]->to_llvm_value(Ctx);
            for (unsigned i = 1; i < test.size(); ++i) {
                arg_values.push_back(test[i]->to_llvm_value(Ctx));
            }
            llvm::ArrayRef<llvm::Value*> args(arg_values);
            auto call_value = builder.CreateCall(checkee, args);
            llvm::dbgs() << *call_value->getType() << "\n";
            llvm::dbgs() << *return_value->getType() << "\n";
            auto cmp_value = builder.CreateICmpEQ(call_value, return_value);

            auto load_result = builder.CreateLoad(result);
            auto trunc_result = builder.CreateTrunc(load_result, llvm::Type::getInt1Ty(Ctx));
            //auto zext_result = builder.CreateZExt(trunc_result, llvm::Type::getInt32Ty(Ctx));
            //auto zext_cmp = builder.CreateZExt(cmp_value, llvm::Type::getInt32Ty(Ctx));
            auto and_res = builder.CreateAnd(trunc_result, cmp_value);
            auto trunc_and = builder.CreateTrunc(and_res, llvm::Type::getInt1Ty(Ctx));
            //auto zext_and = builder.CreateZExt(and_res, llvm::Type::getInt8Ty(Ctx));
            builder.CreateStore(trunc_and, result);

            //insert_call_for_testcase(guard_node.get_function(), result, checkee, test, builder);
        }
    }

    auto load_result = builder.CreateLoad(result);
    auto trunc_result = builder.CreateTrunc(load_result, llvm::Type::getInt1Ty(Ctx));

    llvm::ArrayRef<llvm::Type*> params{llvm::Type::getInt1Ty(Ctx)};
    llvm::FunctionType* function_type = llvm::FunctionType::get(llvm::Type::getVoidTy(Ctx), params, false);
    llvm::Constant *response_func = guard_node.get_function()->getParent()->getOrInsertFunction(
            "response", function_type);
    llvm::ArrayRef<llvm::Value*> args{trunc_result};
    builder.CreateCall(response_func, args);
}

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
    llvm::dbgs() << "Inserting guards to guard function " << F->getName() << "\n";
    llvm::LLVMContext &Ctx = F->getContext();

    llvm::BasicBlock &bb = F->getEntryBlock();
    llvm::IRBuilder<> builder(&bb);
    builder.SetInsertPoint(&bb, ++builder.GetInsertPoint());


    // get or create checker function
    if (!is_sensitive_function) {
        insert_checkee_calls(guard_node, builder);
        llvm::dbgs() << "Inserted\n";
        return;
        
    }
    // Instrument function to cope with ciclic calls
    llvm::dbgs() << "Guard is sensitive function. Inserting cycle handling instructions.\n";
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
    // call checkers
    insert_checkee_calls(guard_node, builder);
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
    FunctionsTestCaseManager::get().collect_test_cases(M, sensitive_function_info);
    for (auto& F : M) {
        if (F.isDeclaration()) {
            continue;
        }
        if (!guards_network.is_guard(&F)) {
            continue;
        }
        auto dom_tree = &getAnalysis<llvm::DominatorTreeWrapperPass>(F).getDomTree();
        auto loop_info = &getAnalysis<llvm::LoopInfoWrapperPass>(F).getLoopInfo();
        insert_guards(guards_network.get_guard_for(&F), dom_tree, loop_info, sensitive_function_info.is_sensitive_function(&F));
    }
    return true;
}

static llvm::RegisterPass<GuardsInserterPass> X("insert-guards","Insert guards in guard functions");
}

