#include <llvm/ADT/STLExtras.h>
#include "llvm/CodeGen/Passes.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "V9CpuTargetMachine.h"
#include "V9Cpu.h"
#include "V9CpuTargetObjectFile.h"
#include "MCTargetDesc/V9CpuABIInfo.h"

using namespace llvm;
extern "C" void LLVMInitializeV9CpuTarget() {
    RegisterTargetMachine<V9CpuITargetMachine> X(TheV9CpuTarget);
}

V9CpuTargetMachine::V9CpuTargetMachine(const Target &T, const Triple &TT,
                                                   StringRef CPU, StringRef FS,
                                                   const TargetOptions &Options,
                                                   Reloc::Model RM, CodeModel::Model CM,
                                                   CodeGenOpt::Level OL)
        : LLVMTargetMachine(T, "e-m:e-p:32:32-f128:64-n32-S64", TT, CPU, FS, Options,
                            RM, CM, OL),
          TLOF(make_unique<V9CpuTargetObjectFile>()),
          ABI(V9CpuABIInfo::computeTargetABI()),
          DefaultSubtarget(TT, CPU, FS, true, *this) { initAsmInfo(); }

V9CpuTargetMachine::~V9CpuTargetMachine() { }

void V9CpuITargetMachine::anchor() { }

V9CpuITargetMachine::V9CpuITargetMachine(const Target &T, const Triple &TT, StringRef CPU, StringRef FS,
                                         const TargetOptions &Options, Reloc::Model RM, CodeModel::Model CM,
                                         CodeGenOpt::Level OL):
        V9CpuTargetMachine(T, TT, CPU, FS, Options, RM, CM, OL) { }