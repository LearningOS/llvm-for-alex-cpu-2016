//===-- V9TargetMachine.cpp - Define TargetMachine for V9 -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//
//===----------------------------------------------------------------------===//

#include "V9TargetMachine.h"
#include "V9TargetObjectFile.h"
#include "V9.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;

extern "C" void LLVMInitializeV9Target() {
  // Register the target.
  RegisterTargetMachine<V9V8TargetMachine> X(TheV9Target);
  RegisterTargetMachine<V9V9TargetMachine> Y(TheV9V9Target);
  RegisterTargetMachine<V9elTargetMachine> Z(TheV9elTarget);
}

static std::string computeDataLayout(const Triple &T, bool is64Bit) {
  // V9 is typically big endian, but some are little.
  std::string Ret = T.getArch() == Triple::sparcel ? "e" : "E";
  Ret += "-m:e";

  // Some ABIs have 32bit pointers.
  if (!is64Bit)
    Ret += "-p:32:32";

  // Alignments for 64 bit integers.
  Ret += "-i64:64";

  // On V9V9 128 floats are aligned to 128 bits, on others only to 64.
  // On V9V9 registers can hold 64 or 32 bits, on others only 32.
  if (is64Bit)
    Ret += "-n32:64";
  else
    Ret += "-f128:64-n32";

  if (is64Bit)
    Ret += "-S128";
  else
    Ret += "-S64";

  return Ret;
}

/// V9TargetMachine ctor - Create an ILP32 architecture model
///
V9TargetMachine::V9TargetMachine(const Target &T, const Triple &TT,
                                       StringRef CPU, StringRef FS,
                                       const TargetOptions &Options,
                                       Reloc::Model RM, CodeModel::Model CM,
                                       CodeGenOpt::Level OL, bool is64bit)
    : LLVMTargetMachine(T, computeDataLayout(TT, is64bit), TT, CPU, FS, Options,
                        RM, CM, OL),
      TLOF(make_unique<V9ELFTargetObjectFile>()),
      Subtarget(TT, CPU, FS, *this, is64bit) {
  initAsmInfo();
}

V9TargetMachine::~V9TargetMachine() {}

namespace {
/// V9 Code Generator Pass Configuration Options.
class V9PassConfig : public TargetPassConfig {
public:
  V9PassConfig(V9TargetMachine *TM, PassManagerBase &PM)
    : TargetPassConfig(TM, PM) {}

  V9TargetMachine &getV9TargetMachine() const {
    return getTM<V9TargetMachine>();
  }

  void addIRPasses() override;
  bool addInstSelector() override;
  void addPreEmitPass() override;
};
} // namespace

TargetPassConfig *V9TargetMachine::createPassConfig(PassManagerBase &PM) {
  return new V9PassConfig(this, PM);
}

void V9PassConfig::addIRPasses() {
  addPass(createAtomicExpandPass(&getV9TargetMachine()));

  TargetPassConfig::addIRPasses();
}

bool V9PassConfig::addInstSelector() {
  addPass(createV9ISelDag(getV9TargetMachine()));
  return false;
}

void V9PassConfig::addPreEmitPass(){
  addPass(createV9DelaySlotFillerPass(getV9TargetMachine()));
}

void V9V8TargetMachine::anchor() { }

V9V8TargetMachine::V9V8TargetMachine(const Target &T, const Triple &TT,
                                           StringRef CPU, StringRef FS,
                                           const TargetOptions &Options,
                                           Reloc::Model RM, CodeModel::Model CM,
                                           CodeGenOpt::Level OL)
    : V9TargetMachine(T, TT, CPU, FS, Options, RM, CM, OL, false) {}

void V9V9TargetMachine::anchor() { }

V9V9TargetMachine::V9V9TargetMachine(const Target &T, const Triple &TT,
                                           StringRef CPU, StringRef FS,
                                           const TargetOptions &Options,
                                           Reloc::Model RM, CodeModel::Model CM,
                                           CodeGenOpt::Level OL)
    : V9TargetMachine(T, TT, CPU, FS, Options, RM, CM, OL, true) {}

void V9elTargetMachine::anchor() {}

V9elTargetMachine::V9elTargetMachine(const Target &T, const Triple &TT,
                                           StringRef CPU, StringRef FS,
                                           const TargetOptions &Options,
                                           Reloc::Model RM, CodeModel::Model CM,
                                           CodeGenOpt::Level OL)
    : V9TargetMachine(T, TT, CPU, FS, Options, RM, CM, OL, false) {}
