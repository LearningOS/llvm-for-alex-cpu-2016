#ifndef ALEX_TARGET_MACHINE_H
#define ALEX_TARGET_MACHINE_H

#include "llvm/Target/TargetMachine.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/Target/TargetFrameLowering.h"
#include "AlexInstrInfo.h"
#include "AlexFrameLowering.h"
#include "AlexISelLowering.h"

#define GET_SUBTARGETINFO_HEADER
#include "AlexGenSubtargetInfo.inc"
namespace llvm {
    extern Target TheAlexTarget;
    class StringRef;

    class AlexTargetMachine;
    class AlexRegisterInfo;
    class AlexTargetLowering;
    class AlexFrameLowering;
    class AlexRegisterInfo;

    class AlexSubtarget : public AlexGenSubtargetInfo {
    public:
        AlexSubtarget(const Triple &TT, const std::string &CPU, const std::string &FS,
                   const AlexTargetMachine *_TM);

        const AlexInstrInfo *getInstrInfo() const override {
            return instrInfo;
        }
        const AlexFrameLowering *getFrameLowering() const override {
            return FrameLowering;
        }
        const AlexRegisterInfo *getRegisterInfo() const override {
            return registerInfo;
        }
        const AlexTargetLowering *getTargetLowering() const override {
            return targetLowering;
        }
        const InstrItineraryData *getInstrItineraryData() const override {
            return instrItineraryData;
        }
    private:
        virtual void anchor() {}
    protected:
        enum AlexArchEnum {
            AlexA1
        };

        AlexArchEnum archVersion;
        Triple targetTriple;

        const AlexTargetMachine *targetMachine;
        const AlexInstrInfo *instrInfo;
        const AlexFrameLowering *FrameLowering;
        const AlexTargetLowering *targetLowering;
        const AlexRegisterInfo *registerInfo;
        InstrItineraryData* instrItineraryData;
    };

    class AlexTargetMachine : public LLVMTargetMachine {

    public:
        AlexTargetMachine(const Target &target, const Triple &targetTripple, StringRef cpu,
                        StringRef fs, const TargetOptions &options,
                        Reloc::Model RM, CodeModel::Model CM, CodeGenOpt::Level OL);
        ~AlexTargetMachine() override {}
//        TargetLoweringObjectFile *getObjFileLowering() const override {
//            return TLOF.get();
//        }
        const AlexSubtarget *getSubtargetImpl(const Function &F) const override {
            return defaultSubtarget;
        }
    private:
        AlexSubtarget* defaultSubtarget;
    };

} // end namespace llvm


#endif // ALEX_TARGET_MACHINE_H