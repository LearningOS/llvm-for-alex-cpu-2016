#ifndef LLVM_LIB_TARGET_ALEX_ALEXINSTRINFO_H
#define LLVM_LIB_TARGET_ALEX_ALEXINSTRINFO_H

#include "AlexRegisterInfo.h"
#include "llvm/Target/TargetInstrInfo.h"

#define GET_INSTRINFO_HEADER
#define GET_INSTRINFO_ENUM
#include "AlexGenInstrInfo.inc"

namespace llvm {

    class AlexInstrInfo : public AlexGenInstrInfo {
        virtual void anchor() { }
        bool expandPostRAPseudo(MachineBasicBlock::iterator MI) const override;
        void expandRetLR(MachineBasicBlock &MBB, MachineBasicBlock::iterator I) const;
    protected:
        const AlexSubtarget *subtarget;
    public:
        explicit AlexInstrInfo(const AlexSubtarget *);

//        static const AlexInstrInfo *create(AlexSubtarget &STI);
//
//        unsigned GetInstSizeInBytes(const MachineInstr *MI) const override {
//            return 4;
//        }
        MachineMemOperand *GetMemOperand(MachineBasicBlock &MBB, int FI,
                                                        unsigned Flag) const;

        void adjustStackPtr(unsigned SP, int64_t Amount,
                                           MachineBasicBlock &MBB,
                                           MachineBasicBlock::iterator I) const;
        void storeRegToStackSlot(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
                                unsigned SrcReg, bool isKill, int FI,
                                const TargetRegisterClass *RC, const TargetRegisterInfo *TRI) const override;
        void loadRegFromStackSlot(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
                                  unsigned DestReg, int FI, const TargetRegisterClass *RC,
                                  const TargetRegisterInfo *TRI) const override;
    };
}

#endif