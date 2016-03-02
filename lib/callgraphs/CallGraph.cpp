
#include "CallGraph.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Support/raw_ostream.h"

#include <deque>
#include <iterator>

using namespace llvm;
using namespace callgraphs;


char CallGraphPass::ID = 0;
char WeightedCallGraphPass::ID = 0;

RegisterPass<WeightedCallGraphPass> X{"weightedcg",
                                "construct a weighted call graph of a module"};
RegisterPass<CallGraphPass> Y{"callgraph",
                            "construct a call grpah of the module"};

bool
CallGraphPass::runOnModule(Module &m) {
  // A good design might be to use the CallGraphPass to compute the call graph
  // and then use that call graph for computing and printing the weights in
  // WeightedCallGraph.
  
  for( auto &f : m ){
    if(f.getName().str() == "llvm.dbg.declare")
      continue;
    funcs.insert( std::make_pair( &f, FunctionInfo(&f) ));
  }
  
  
  for( auto &f : m ){
    if( f.getName() == "llvm.dbg.declare"){
      continue;
    }
    FunctionInfo fun(&f);
    funcList.push_back(fun);
    for( auto &bb : f ){
      for( auto &i : bb){
        handleInstruction( llvm::CallSite(&i), &fun, m );
      }
    }
  }

  return false;
}

void
CallGraphPass::handleInstruction(llvm::CallSite cs, callgraphs::FunctionInfo *fun, llvm::Module &m){
  // Check whether the instruction is actually a call
  if (!cs.getInstruction()) {
    return;
  }

  // Check whether the called function is directly invoked
  auto called = dyn_cast<Function>(cs.getCalledValue()->stripPointerCasts());
  if (!called) {
    for(auto &f : m){
      if(f.hasAddressTaken()){
        
        bool match = true;
        std::vector< Type* > argslist;
        for (Use &U : cs.getInstruction()->operands()) {
          Value *v = U.get();
          argslist.push_back( v->getType() ); 
        }
        
        llvm::Function::ArgumentListType &alt = f.getArgumentList();
        int j = 0;
        for( auto &a : alt){
          if( a.getType() != argslist[j++]){
            match = false;
         }
        }
        
        if( argslist.size() > (j+1) && !f.isVarArg() ){
          match = false;
        }
        
        if(match){
          DILocation *Loc = cs.getInstruction()->getDebugLoc();
          callgraphs::CallInfo ci( &f, Loc->getLine() , Loc->getFilename(),
                                  funcs.find( fun->getFunction() )->second.callCount);
          funcs.find( &f )->second.weight++;
          funcs.find( fun->getFunction() )->second.directCalls.push_back( ci );
        }
        
        
      }
    }
    funcs.find( fun->getFunction() )->second.callCount++;
    return;
  }

  if(called->getName() == "llvm.dbg.declare")
    return;
    
  // Direct Calls heres
  DILocation *Loc = cs.getInstruction()->getDebugLoc();
  callgraphs::CallInfo ci(called, Loc->getLine() , Loc->getFilename(), 
    funcs.find( fun->getFunction() )->second.callCount  );
  funcs.find( called )->second.weight++;
  funcs.find( fun->getFunction() )->second.directCalls.push_back( ci );
  funcs.find( fun->getFunction() )->second.callCount++;
  
}

// For an analysis pass, runOnModule should perform the actual analysis and
// compute the results. Any actual output, however, is produced separately.
bool
WeightedCallGraphPass::runOnModule(Module &m) {
  // The results of the call graph pass can be extracted and used here.
  auto &cgPass = getAnalysis<CallGraphPass>();

  // TODO Use the call graph to compute function weights.

  return false;
}


// Output for a pure analysis pass should happen in the print method.
// It is called automatically after the analysis pass has finished collecting
// its information.
void
WeightedCallGraphPass::print(raw_ostream &out, const Module *m) const {
  auto &cgPass = getAnalysis<CallGraphPass>();

  
  // Print out all functions
  for ( auto &kvPair : cgPass.funcs ) {
    FunctionInfo fi = kvPair.second;
    out << fi.getFunction()->getName() << "," << fi.weight;

    unsigned siteID = 0;
    for ( auto ci : fi.directCalls ) {
      out << "," << siteID << "," << ci.filename << "," << ci.lineNum;
      ++siteID;
    }
    out << "\n";
  }
  

  // Separate functions and edges by a blank line
  out << "\n";

  for ( auto &kvPair : cgPass.funcs ) {
    FunctionInfo fi = kvPair.second;
    for ( auto ci : fi.directCalls ) {
      out << fi.getFunction()->getName() << "," << ci.callSiteNum << 
          "," << ci.getFunction()->getName() << "\n";
    }
  }
}

