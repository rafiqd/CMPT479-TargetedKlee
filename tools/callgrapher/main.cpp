
#include "llvm/AsmParser/Parser.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"

#include <memory>
#include <string>

#include "CallGraph.h"


using namespace std;
using namespace llvm;
using llvm::legacy::PassManager;


// TODO: This is a temporary placeholder until make_unique ships widely.
template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>{new T(forward<Args>(args)...)};
}


namespace {


cl::opt<string>
inPath{cl::Positional,
       cl::desc("<Module to analyze>"),
       cl::value_desc("bitcode filename"), cl::Required};


template<typename T>
struct CallGraphPrinter : public ModulePass {
  static char ID;
  raw_ostream &out;

  CallGraphPrinter(raw_ostream &out)
    : ModulePass{ID},
      out(out)
      { }

  bool
  runOnModule(Module &m) override {
    getAnalysis<T>().print(out, &m);
    return false;
  }

  void
  getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<T>();
    AU.setPreservesAll();
  }
};

template<typename T>
char CallGraphPrinter<T>::ID = 0;


}


int
main (int argc, char **argv, const char **env) {
  // This boilerplate provides convenient stack traces and clean LLVM exit
  // handling. It also initializes the built in support for convenient
  // command line option handling.
  sys::PrintStackTraceOnErrorSignal();
  llvm::PrettyStackTraceProgram X{argc, argv};
  llvm_shutdown_obj shutdown;
  cl::ParseCommandLineOptions(argc, argv);

  // Construct an IR file from the filename passed on the command line.
  LLVMContext &context = getGlobalContext();
  SMDiagnostic err;
  unique_ptr<Module> module = parseIRFile(inPath.getValue(), err, context);

  if (!module.get()) {
    errs() << "Error reading bitcode file.\n";
    err.print(argv[0], errs());
    return -1;
  }

  // Build up all of the passes that we want to run on the module.
  PassManager pm;
  pm.add(new callgraphs::CallGraphPass);
  pm.add(new CallGraphPrinter<callgraphs::WeightedCallGraphPass>(outs()));
  pm.add(new callgraphs::WeightedCallGraphPass);
  pm.run(*module);

  return 0;
}

