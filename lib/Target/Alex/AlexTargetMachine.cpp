#include "AlexTargetMachine.h"
#include "AlexISelDAGToDAG.h"
#include "AlexTargetObjectFile.h"
#include "AlexRegisterInfo.h"
#include "AlexInstrInfo.h"
#include "AlexFrameLowering.h"
#include "AlexISelLowering.h"
#include <llvm/ADT/STLExtras.h>
#include "llvm/CodeGen/Passes.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"

using namespace llvm;
Target llvm::TheAlexTarget;
extern "C" void LLVMInitializeAlexTarget() {
    RegisterTargetMachine<AlexTargetMachine> x(TheAlexTarget);
}
extern "C" void LLVMInitializeAlexTargetInfo() {
    // Register the target.
    RegisterTarget<Triple::alex, /*has JIT?*/ false> X(TheAlexTarget, "alex", "Alex");
}
#define DEBUG_TYPE "Alex-subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "AlexGenSubtargetInfo.inc"

AlexSubtarget::AlexSubtarget(const Triple &TT, const std::string &CPU, const std::string &FS,
                    const AlexTargetMachine *_TM) :
              AlexGenSubtargetInfo(TT, CPU, FS),
              targetMachine(_TM),
              targetTriple(TT),
              instrInfo(new AlexInstrInfo(this)),
              FrameLowering(new AlexFrameLowering(this)),
              instrItineraryData(nullptr)

{
    registerInfo = new AlexRegisterInfo(this);
    targetLowering = new AlexTargetLowering(_TM, this, registerInfo);
}

namespace {
    class AlexPassConfig : public TargetPassConfig {
    public:
        AlexPassConfig(AlexTargetMachine *TM, PassManagerBase &PM)
        : TargetPassConfig(TM, PM) {}

        AlexTargetMachine &getAlexTargetMachine() const {
            return getTM<AlexTargetMachine>();
        }

        const AlexSubtarget &getAlexSubtarget() const {
            return *getAlexTargetMachine().getSubtargetImpl();
        }

        bool addInstSelector() override {
            addPass(new AlexDAGToDAGISel(getAlexTargetMachine()));
            return false;
        }
    };
}

TargetPassConfig *AlexTargetMachine::createPassConfig(PassManagerBase &PM) {
    return new AlexPassConfig(this, PM);
}


AlexTargetMachine::AlexTargetMachine(const Target &target, const Triple &targetTripple,
                                                   StringRef cpu, StringRef fs,
                                                   const TargetOptions &options,
                                                   Reloc::Model RM, CodeModel::Model CM,
                                                   CodeGenOpt::Level OL)
        : LLVMTargetMachine(target, "e-m:e-p:32:32-f128:64-n32-S64", targetTripple, cpu, fs, options, RM, CM, OL),
          defaultSubtarget(new AlexSubtarget(targetTripple, cpu, fs, this)),
          targetLoweringObjectFile(new AlexTargetObjectFile())
{
    initAsmInfo();
}