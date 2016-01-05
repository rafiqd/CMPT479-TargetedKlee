
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


bool
CallGraphPass::runOnModule(Module &m) {
  // A good design might be to use the CallGraphPass to compute the call graph
  // and then use that call graph for computing and printing the weights in
  // WeightedCallGraph.

  // TODO: Compute the call graph

  return false;
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
  for (/* Iterate through all functions */) {
    out << /* function name */ << "," << /* Function Weight*/;

    unsigned siteID = 0;
    for (/* For each call site in the function */) {
      out << "," << siteID << "," << /* file name */ << "," << /*Line Number*/;
      ++siteID;
    }
    out << "\n";
  }

  // Separate functions and edges by a blank line
  out << "\n";

  // Print out all edges
  for (/* Iterate through all functions */) {
    unsigned siteID = 0;
    for (/* For each call site in the function */) {
      for (/* For each possible call target */) {
        out << /* Caller Function Name */ << "," << siteID << ","
            << /* Target function name*/ << "\n";
      }
      ++siteID;
    }
  }
}

