#include "AlexTargetMachine.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;
Target llvm::TheAlexTarget;

extern "C" void LLVMInitializeAlexTargetInfo() {
  // Register the target.
  RegisterTarget<Triple::alex, /*has JIT?*/ false> X(TheAlexTarget, "alex", "Alex");
}