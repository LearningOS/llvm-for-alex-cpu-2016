#include <llvm/ADT/STLExtras.h>
#include "llvm/CodeGen/Passes.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "V9CpuTargetMachine.h"
#include "V9Cpu.h"
#include "V9CpuTargetObjectFile.h"
#include "MCTargetDesc/V9CpuABIInfo.h"
#include "V9CpuSEISelDAGToDAG.h"

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

const V9CpuSubtarget *V9CpuTargetMachine::getSubtargetImpl(const Function &F) const {
    Attribute CPUAttr = F.getFnAttribute("target-cpu");
    Attribute FSAttr = F.getFnAttribute("target-features");

    std::string CPU = !CPUAttr.hasAttribute(Attribute::None)
                      ? CPUAttr.getValueAsString().str()
                      : TargetCPU;
    std::string FS = !FSAttr.hasAttribute(Attribute::None)
                     ? FSAttr.getValueAsString().str()
                     : TargetFS;
    resetTargetOptions(F);
    auto I = llvm::make_unique<V9CpuSubtarget>(TargetTriple, CPU, FS, true,
                                         *this);
    return I.get();
}


void V9CpuITargetMachine::anchor() { }

V9CpuITargetMachine::V9CpuITargetMachine(const Target &T, const Triple &TT, StringRef CPU, StringRef FS,
                                         const TargetOptions &Options, Reloc::Model RM, CodeModel::Model CM,
                                         CodeGenOpt::Level OL):
        V9CpuTargetMachine(T, TT, CPU, FS, Options, RM, CM, OL) { }

namespace {
//@Cpu0PassConfig {
/// Cpu0 Code Generator Pass Configuration Options.
    class V9CpuPassConfig : public TargetPassConfig {
    public:
        V9CpuPassConfig(V9CpuTargetMachine *TM, PassManagerBase &PM)
                : TargetPassConfig(TM, PM) {}

        V9CpuTargetMachine &getV9CpuTargetMachine() const {
            return getTM<V9CpuTargetMachine>();
        }

        const V9CpuSubtarget &getV9CpuSubtarget() const {
            return *getV9CpuTargetMachine().getSubtargetImpl();
        }
        void addIRPasses() override { }
        bool addInstSelector() override {
            addPass(createV9CpuSEISelDag(getV9CpuTargetMachine()));
            return false;
        }
    };
} // namespace

TargetPassConfig *V9CpuTargetMachine::createPassConfig(PassManagerBase &PM) {
    return new V9CpuPassConfig(this, PM);
}