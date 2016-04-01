#include "llvm/CodeGen/Passes.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/TargetRegistry.h"
#include "V9CpuTargetMachine.h"

namespace llvm {

    Target TheV9CpuTarget;

    extern "C" void LLVMInitializeV9CpuTargetInfo() {
      // Register the target.
      RegisterTargetMachine<Triple::v9cpu, V9CpuSeriesTargetMachine> X(llvm::TheV9CpuTarget);
    }
}

using namespace llvm;
V9CpuSeriesTargetMachine::V9CpuSeriesTargetMachine(const Target &T, const Triple &TT,
                                                   StringRef CPU, StringRef FS,
                                                   const TargetOptions &Options,
                                                   Reloc::Model RM, CodeModel::Model CM,
                                                   CodeGenOpt::Level OL)
        : LLVMTargetMachine(T, "e-m:e-p:32:32-f128:64-n32-S64", TT, CPU, FS, Options,
                            RM, CM, OL) { }

V9CpuSeriesTargetMachine::~V9CpuSeriesTargetMachine() { }
