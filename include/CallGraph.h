
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

namespace callgraphs {


struct CallGraphPass : public llvm::ModulePass {

  static char ID;

public:
  CallGraphPass()
    : ModulePass(ID)
      { }

  void
  getAnalysisUsage(llvm::AnalysisUsage &au) const override {
    au.setPreservesAll();
  }

  bool runOnModule(llvm::Module &m) override;
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
