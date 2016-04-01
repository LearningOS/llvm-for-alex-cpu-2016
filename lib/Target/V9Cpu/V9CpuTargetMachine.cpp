#include "V9TargetMachine.h"
#include "V9TargetObjectFile.h"
#include "V9.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;

extern "C" void LLVMInitializeV9CpuTarget() {
  // Register the target.
  RegisterTargetMachine<V9TargetMachine> X(TheV9Target);
}

static std::string computeDataLayout(const Triple &T, bool is64Bit) {
    std::string Ret = "e-m:e-p:32:32-f128:64-n32-S64";
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
