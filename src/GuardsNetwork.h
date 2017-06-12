#pragma once

#include "llvm/Pass.h"

#include "dot_interfaces.h"

#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace llvm {
class Function;
class Module;
}

namespace result_checking {

class SensitiveFunctionInformation;
//class FunctionDominanceTree;

class GuardNetwork
{
public:
    using FunctionSet = std::unordered_set<llvm::Function*>;
    using Functions = std::vector<llvm::Function*>;

public:
    class GuardNode
    {
    public:
        GuardNode(llvm::Function* f);

    public:
        void add_checkee(llvm::Function* f);
        const FunctionSet& get_checkees() const;
        FunctionSet& get_checkees();
        llvm::Function* get_function() const;

    private:
        llvm::Function* function;
        FunctionSet checkees;
    };

public:
    GuardNetwork() = default;
    GuardNetwork(const GuardNetwork&) = delete;
    GuardNetwork& operator =(const GuardNetwork&) = delete;

public:
    void set_connectivity_level(unsigned con_level)
    {
        connectivity_level = con_level;
    }

    bool is_empty() const;
    bool is_guard(llvm::Function* f) const;
    const GuardNode& get_guard_for(llvm::Function* f) const;
    GuardNode& get_guard_for(llvm::Function* f);
    const std::unordered_map<llvm::Function*, GuardNode> get_guard_functions() const;

    void build(const FunctionSet& module_functions,
               const SensitiveFunctionInformation& functions_info);
               //const FunctionDominanceTree& dominance_tree);

private:
    GuardNode& get_or_create_guard_for(llvm::Function* F);
    void create_guards(const Functions& guards, llvm::Function* checkee);

private:
    unsigned connectivity_level;
    std::unordered_map<llvm::Function*, GuardNode> guard_functions;
};

class GuardNetworkCreatorPass : public llvm::ModulePass
{
public:
    static char ID;

    GuardNetworkCreatorPass()
        : llvm::ModulePass(ID)
    {
    }

public:
    void getAnalysisUsage(llvm::AnalysisUsage& AU) const override;
    bool runOnModule(llvm::Module& M) override;

public:
    const GuardNetwork& get_guards_network() const
    {
        return guard_network;
    }

    GuardNetwork& get_guards_network()
    {
        return guard_network;
    }

private:
    GuardNetwork guard_network;
};

class GuardNetworkCreatorPrinterPass : public llvm::ModulePass
{
public:
    class FunctionWrapper : public dot::DotGraphNodeType
    {
    public:
        FunctionWrapper(llvm::Function* f);

        virtual std::string get_id() const override;
        virtual std::string get_label() const override;

    private:
        llvm::Function* function;
    };

    class GuardNodeWrapper : public dot::DotGraphNodeType
    {
    public:
        GuardNodeWrapper(const GuardNetwork::GuardNode& node);

        using DotGraphNodeType_ptr = dot::DotGraphNodeType::DotGraphNodeType_ptr;
        virtual std::string get_id() const override;
        virtual std::vector<DotGraphNodeType_ptr> get_connections() const override;
        virtual std::string get_label() const override;

    private:
        const GuardNetwork::GuardNode& guard_node;
    };
public:
    static char ID;

    GuardNetworkCreatorPrinterPass()
        : llvm::ModulePass(ID)
    {
    }
    
public:
    void getAnalysisUsage(llvm::AnalysisUsage& AU) const override;
    bool runOnModule(llvm::Module& M) override;
   
private:
    using GraphNodeType_ptr = std::shared_ptr<dot::DotGraphNodeType>;
    std::vector<GraphNodeType_ptr> guard_wrappers;
};

} // namespace result_checking

