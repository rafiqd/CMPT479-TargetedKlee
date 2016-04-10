
#include "CallGraph.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Support/raw_ostream.h"

#include <deque>
#include <iterator>

#include <fstream>
#include <sstream>
#include <iostream>
#include <string>

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
  if( Loc ){
    callgraphs::CallInfo ci(called, Loc->getLine() , Loc->getFilename(), 
    funcs.find( fun->getFunction() )->second.callCount  );
    funcs.find( fun->getFunction() )->second.directCalls.push_back( ci );
    funcs.find( fun->getFunction() )->second.callCount++;
    fun->filename = Loc->getFilename();
  }else{
    callgraphs::CallInfo ci(called, 0, "unknown", 
    funcs.find( fun->getFunction() )->second.callCount  );
    funcs.find( called )->second.weight++;
    funcs.find( fun->getFunction() )->second.directCalls.push_back( ci );
    funcs.find( fun->getFunction() )->second.callCount++;
    fun->filename = "unknown";
  }
  
}

// For an analysis pass, runOnModule should perform the actual analysis and
// compute the results. Any actual output, however, is produced separately.
bool
WeightedCallGraphPass::runOnModule(Module &m) {
  // The results of the call graph pass can be extracted and used here.
  auto &cgPass = getAnalysis<CallGraphPass>();
  
  for( auto &f : m ){
    errs() << "\n";
    if( f.getName() == "llvm.dbg.declare"){
    continue;
    }
    for( auto &bb : f ){
      for( auto &i : bb){
        //errs() << i << "\n";
        if( i.hasMetadata() && i.getMetadata("weight") ){
          //errs() << "i has meta data --";
          //errs() << i.getMetadata("weight")->getNumOperands() << "--";
          //errs() << cast<MDString>(i.getMetadata("weight")->getOperand(0))->getString() << "--\n";
        }
          
        
        llvm::CallSite cs( (llvm::Instruction*) &i ); 
        if( cs.isCall() ){
          auto found = cgPass.funcs.find( cs.getCalledFunction() );
          if( found != cgPass.funcs.end() ){
            int weight = found->second.bugweight;
            for( auto &ii : *i.getParent() ){
              auto &context = ii.getContext();
              auto *str = MDString::get(context, StringRef( std::to_string( weight ) ) );
              auto *node = MDNode::get(context, str);
              ii.setMetadata(StringRef("weight"), node);
            }
          }
        }
        
      }
    }
  }

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
#if FALSE
  for ( auto &kvPair : cgPass.funcs ) {
    FunctionInfo fi = kvPair.second;
    for ( auto ci : fi.directCalls ) {
      out << fi.getFunction()->getName() << "," << ci.callSiteNum << 
          "," << ci.getFunction()->getName() << "\n";
    }
  }
#endif
  
  std::ifstream file;
  file.open ("hotspots");
  
  std::map<std::string,unsigned> bugFrequency;
  if( file.is_open() ){
    std::string line;
    while ( std::getline(file,line) ){
      std::stringstream ss(line);
      std::string fname;
      unsigned freq;
      ss >> fname >> freq;
      errs() << "fname:" << fname << " freq:" << freq << "\n";
      std::pair<std::string,unsigned> p( fname, freq);
      bugFrequency.insert( p );
    }
    file.close();
  }
  

  for ( auto &kvPair : cgPass.funcs ){
    FunctionInfo fi = kvPair.second;
    // if this function is in a hotspot file  fi =  +(3*num of bug frequency) to weight here
    out << fi.getFunction()->getName() << "\n";
    auto depth1 = bugFrequency.find( fi.filename );
    if( depth1 != bugFrequency.end() ){
      fi.bugweight += 3 * ( depth1->second );
      errs() << "d1\n";
    }
    
    for( auto ci: fi.directCalls ){
      auto found = cgPass.funcs.find( ci.getFunction() );
      if( found != cgPass.funcs.end() ){
        out << "\t" << found->second.getFunction()->getName() << "\n";
        // if found->second.getfunction is in a hotspot file fi = +(2*num of bug frequency ) to weight
        auto depth2 = bugFrequency.find( found->second.filename );
        if( depth2 != bugFrequency.end() ){
          fi.bugweight += 2 * ( depth2->second );
          errs() << "d2\n";
        }
        
        for( auto ci2 : found->second.directCalls ){
          // if in hotspot, fi = +(1*num of bug freq) to weight
          auto found2 = cgPass.funcs.find( ci2.getFunction() );
          if( found2 != cgPass.funcs.end() ){
            auto depth3 = bugFrequency.find( found2->second.filename );
            if( depth3 != bugFrequency.end() ){
              fi.bugweight += depth3->second;  
              errs() << "d3\n";
            }
          }
           out << "\t\t" << ci2.getFunction()->getName() << "\n";
        }
        
      }
    }
  }
  
  for( auto &kvPair : cgPass.funcs ){
    FunctionInfo fi = kvPair.second;
    out << fi.getFunction()->getName() << " has weight:" << fi.bugweight << "\n";
  }


  

}

