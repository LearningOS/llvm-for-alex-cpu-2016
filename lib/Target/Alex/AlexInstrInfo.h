#ifndef LLVM_LIB_TARGET_ALEX_ALEXINSTRINFO_H
#define LLVM_LIB_TARGET_ALEX_ALEXINSTRINFO_H

#include "AlexRegisterInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/Target/TargetInstrInfo.h"

#define GET_INSTRINFO_HEADER
#define GET_INSTRINFO_ENUM
#include "AlexGenInstrInfo.inc"

namespace llvm {

    class AlexInstrInfo : public AlexGenInstrInfo {
        virtual void anchor() { }
    protected:
        const AlexSubtarget *subtarget;
    public:
        explicit AlexInstrInfo(const AlexSubtarget *);

//        static const AlexInstrInfo *create(AlexSubtarget &STI);
//
//        unsigned GetInstSizeInBytes(const MachineInstr *MI) const override {
//            return 4;
//        }
//        virtual void adjustStackPtr(unsigned SP, int64_t Amount,
//                                    MachineBasicBlock &MBB,
//                                    MachineBasicBlock::iterator I) const = 0;
    };
}

#endif