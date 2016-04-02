#include "V9CpuTargetMachine.h"
#include "V9Cpu.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/IR/Module.h"

using namespace llvm;
Target llvm::TheV9CpuTarget;

extern "C" void LLVMInitializeV9CpuTargetInfo() {
    // Register the target.
    RegisterTarget<Triple::v9cpu, /*has JIT?*/ false> X(TheV9CpuTarget, "v9cpu", "V9Cpu");
}