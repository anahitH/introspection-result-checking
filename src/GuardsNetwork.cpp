#include "GuardsNetwork.h"

#include "SensitiveFunctionMarkPass.h"
//#include "FunctionDominanceTree.h"
#include "DotPrinter.h"

#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/CommandLine.h"

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

#include <ctime>
#include <list>
#include <sstream>

namespace result_checking {

namespace {

using Functions = GuardNetwork::Functions;

/*
Functions get_function_dominators(llvm::Function* function, const FunctionDominanceTree& dominator_tree)
{
    Functions dominators;

    auto dom_node = dominator_tree.get_function_dominators(function);
    assert(dom_node != nullptr);
    auto immediate_doms = dom_node->get_dominators();
    if (immediate_doms.empty()) {
        return dominators;
    }

    std::list<FunctionDominanceTree::DomNodeT> domnode_list;
    domnode_list.insert(domnode_list.begin(), immediate_doms.begin(), immediate_doms.end());
    while (!domnode_list.empty()) {
        auto node = domnode_list.back();
        domnode_list.pop_back();
        dominators.push_back(node->get_function());
        immediate_doms = node->get_dominators();
        if (!immediate_doms.empty()) {
            domnode_list.insert(domnode_list.begin(), immediate_doms.begin(), immediate_doms.end());
        }
    }
    return dominators;
}
*/
Functions get_random_functions_from_list(Functions function_list, unsigned connectivity_level)
{
    Functions functions;
    srand(time(NULL));
    // hope function_list size is not as big as long unsigned
    unsigned size = function_list.size();
    connectivity_level = std::min(size, connectivity_level);
    while (connectivity_level-- != 0) {
        unsigned index = rand() % size;
        functions.push_back(function_list[index]);
        function_list[index] = function_list.back();
        function_list.pop_back();
        --size;
    }

    return functions;
}

Functions get_random_functions_from_list(GuardNetwork::FunctionSet functions, unsigned connectivity_level, const Functions& filter)
{
    for (auto f : filter) {
        functions.erase(f);
    }
    Functions function_list(functions.begin(), functions.end());
    return get_random_functions_from_list(function_list, connectivity_level);
}

}

GuardNetwork::GuardNode::GuardNode(llvm::Function* f)
    : function(f)
{
}

void GuardNetwork::GuardNode::add_checkee(llvm::Function* f)
{
    assert(function != f);
    checkees.insert(f);
}

const GuardNetwork::FunctionSet& GuardNetwork::GuardNode::get_checkees() const
{
    return checkees;
}

GuardNetwork::FunctionSet& GuardNetwork::GuardNode::get_checkees()
{
    return checkees;
}

llvm::Function* GuardNetwork::GuardNode::get_function() const
{
    return function;
}

bool GuardNetwork::is_empty() const
{
    return guard_functions.empty();
}

bool GuardNetwork::is_guard(llvm::Function* f) const
{
    return guard_functions.find(f) != guard_functions.end();
}

const GuardNetwork::GuardNode& GuardNetwork::get_guard_for(llvm::Function* f) const
{
    return const_cast<GuardNetwork*>(this)->get_guard_for(f);
}

GuardNetwork::GuardNode& GuardNetwork::get_guard_for(llvm::Function* f)
{
    auto pos = guard_functions.find(f);
    assert(pos != guard_functions.end());
    return pos->second;
}

const std::unordered_map<llvm::Function*, GuardNetwork::GuardNode> GuardNetwork::get_guard_functions() const
{
    return guard_functions;
}

void GuardNetwork::build(const FunctionSet& module_functions,
                         const SensitiveFunctionInformation& functions_info)
                         //const FunctionDominanceTree& dominance_tree)
{
    const auto& sensitive_functions = functions_info.get_sensitive_functions();
    for (const auto& sens_function : sensitive_functions) {
        //auto dominators = get_function_dominators(sens_function, dominance_tree);
        //const auto& dominator_guards = get_random_functions_from_list(dominators, connectivity_level);
        //create_guards(dominator_guards, sens_function);
        //auto remaining_guards_num = connectivity_level - dominator_guards.size();
        //if (remaining_guards_num != 0) {
            //dominators.push_back(sens_function); // to filter itself too
            Functions functions;
            functions.push_back(sens_function);
            const auto& random_guards = get_random_functions_from_list(module_functions, connectivity_level, functions);
            create_guards(random_guards, sens_function);
        //}
    }
}

GuardNetwork::GuardNode& GuardNetwork::get_or_create_guard_for(llvm::Function* F)
{
    auto res = guard_functions.insert(std::make_pair(F, GuardNode(F)));
    return res.first->second;
}

void GuardNetwork::create_guards(const Functions& guards, llvm::Function* checkee)
{
    for (auto guard_f : guards) {
        auto& guard = get_or_create_guard_for(guard_f);
        guard.add_checkee(checkee);
    }
}

static llvm::cl::opt<unsigned> connectivity_level("connectivity-level",
                                                  llvm::cl::desc("Specify connectivity level for guard network"),
                                                  llvm::cl::value_desc("connectivity_level"));

char GuardNetworkCreatorPass::ID = 0;

void GuardNetworkCreatorPass::getAnalysisUsage(llvm::AnalysisUsage& AU) const
{
    AU.setPreservesAll();
    AU.addRequired<SensitiveFunctionMarkPass>();
    //AU.addRequired<FunctionDominanceTreePass>();
}

bool GuardNetworkCreatorPass::runOnModule(llvm::Module& M)
{
    const auto& sensitive_function_info = getAnalysis<SensitiveFunctionMarkPass>().get_sensitive_functions_info();
    //const auto& function_dom_info = getAnalysis<FunctionDominanceTreePass>().get_dominance_tree();
    guard_network.set_connectivity_level(connectivity_level);

    //SensitiveFunctionInformation sensitive_function_info;
    //sensitive_function_info.collect_sensitive_functions(InputFilename, M);
    
    const auto& module_function_list = M.getFunctionList();
    GuardNetwork::FunctionSet module_functions;
    std::for_each(module_function_list.begin(), module_function_list.end(),
                  [&module_functions] (const llvm::Function& f) { 
                      if (!f.isDeclaration()) {
                            module_functions.insert(const_cast<llvm::Function*>(&f));
                      }
                    });

    guard_network.build(module_functions, sensitive_function_info);
    return false;
}


GuardNetworkCreatorPrinterPass::FunctionWrapper::FunctionWrapper(llvm::Function* f)
    : function(f)
{
}

std::string GuardNetworkCreatorPrinterPass::FunctionWrapper::get_id() const
{
    std::ostringstream address;
    address << (void const *)function;
    std::string name = address.str();
    return name;
}

std::string GuardNetworkCreatorPrinterPass::FunctionWrapper::get_label() const
{
    return function->getName();
}

GuardNetworkCreatorPrinterPass::GuardNodeWrapper::GuardNodeWrapper(const GuardNetwork::GuardNode& node)
    : guard_node(node)
{
}

std::string GuardNetworkCreatorPrinterPass::GuardNodeWrapper::get_id() const
{
    std::ostringstream address;
    address << (void const *)(guard_node.get_function());
    std::string name = address.str();
    return name;
}

std::vector<GuardNetworkCreatorPrinterPass::GuardNodeWrapper::DotGraphNodeType_ptr>
GuardNetworkCreatorPrinterPass::GuardNodeWrapper::get_connections() const
{
    std::vector<DotGraphNodeType_ptr> connections;
    for (const auto& conn : guard_node.get_checkees()) {
        connections.push_back(DotGraphNodeType_ptr(new GuardNetworkCreatorPrinterPass::FunctionWrapper(conn)));
    }
    return connections;
}

std::string GuardNetworkCreatorPrinterPass::GuardNodeWrapper::get_label() const
{
    return guard_node.get_function()->getName();
}

char GuardNetworkCreatorPrinterPass::ID = 0;

void GuardNetworkCreatorPrinterPass::getAnalysisUsage(llvm::AnalysisUsage& AU) const
{
    AU.setPreservesAll();
    AU.addRequired<GuardNetworkCreatorPass>();
}

bool GuardNetworkCreatorPrinterPass::runOnModule(llvm::Module& M)
{
    const auto& guards = getAnalysis<GuardNetworkCreatorPass>().get_guards_network().get_guard_functions();
    for (const auto& guard : guards) {
        guard_wrappers.push_back(GraphNodeType_ptr(new GuardNodeWrapper(guard.second)));
    }
    dot::DotPrinter printer;
    printer.set_graph_name("guards_network");
    printer.set_graph_label("Guards Network");
    printer.print(guard_wrappers);
    return false;
}
 
static llvm::RegisterPass<GuardNetworkCreatorPass> X("guard-network","Creates guards' network for given functions");
static llvm::RegisterPass<GuardNetworkCreatorPrinterPass> Y("dot-guard-network","Prints guards' network for given functions");

}

