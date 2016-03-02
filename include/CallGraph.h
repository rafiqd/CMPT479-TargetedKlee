
#ifndef CALLGRAPH_H
#define CALLGRAPH_H

#include "llvm/IR/CallSite.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"

#include <unordered_map>
#include <unordered_set>
#include <deque>
#include <vector>



namespace callgraphs {

struct CallInfo{
  public:
    CallInfo( llvm::Function *n, unsigned line, llvm::StringRef fname,
              unsigned csnum): name(n), lineNum(line), filename(fname), 
              callSiteNum(csnum) {}
    llvm::Function* getFunction(){ return name; }
  
    llvm::Function *name;
    unsigned lineNum;
    llvm::StringRef filename;
    unsigned callSiteNum;
};

struct FunctionInfo{
  public:
    std::vector< CallInfo > directCalls;
    std::vector< CallInfo > possibleCalls;    
    unsigned int callCount;
    unsigned int weight;
    llvm::Function *name;
    
    FunctionInfo( llvm::Function *n ): name(n), callCount(0), weight(0) {}
    llvm::Function* getFunction(){ return name; };

};


struct CallGraphPass : public llvm::ModulePass {

  static char ID;

public:

  std::vector< FunctionInfo > funcList;
  llvm::DenseMap< llvm::Function*, FunctionInfo > funcs;
  CallGraphPass()
    : ModulePass(ID)
      { }

  void
  getAnalysisUsage(llvm::AnalysisUsage &au) const override {
    au.setPreservesAll();
  }

  bool runOnModule(llvm::Module &m) override;
  void handleInstruction(llvm::CallSite cs, FunctionInfo *fun, llvm::Module &m);

};


struct WeightedCallGraphPass : public llvm::ModulePass {

  static char ID;

  WeightedCallGraphPass()
    : ModulePass(ID)
      { }

  void
  getAnalysisUsage(llvm::AnalysisUsage &au) const override {
    au.setPreservesAll();
    au.addRequired<CallGraphPass>();
  }

  void print(llvm::raw_ostream &out, const llvm::Module *m) const override;

  bool runOnModule(llvm::Module &m) override;
};


}


#endif
