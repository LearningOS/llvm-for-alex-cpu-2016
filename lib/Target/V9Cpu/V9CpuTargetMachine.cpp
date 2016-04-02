#include "llvm/CodeGen/Passes.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/TargetRegistry.h"
#include "V9CpuTargetMachine.h"
#include "V9Cpu.h"

using namespace llvm;
extern "C" void LLVMInitializeV9CpuTarget() {
}

V9CpuSeriesTargetMachine::V9CpuSeriesTargetMachine(const Target &T, const Triple &TT,
                                                   StringRef CPU, StringRef FS,
                                                   const TargetOptions &Options,
                                                   Reloc::Model RM, CodeModel::Model CM,
                                                   CodeGenOpt::Level OL)
        : LLVMTargetMachine(T, "e-m:e-p:32:32-f128:64-n32-S64", TT, CPU, FS, Options,
                            RM, CM, OL) { }

V9CpuSeriesTargetMachine::~V9CpuSeriesTargetMachine() { }
