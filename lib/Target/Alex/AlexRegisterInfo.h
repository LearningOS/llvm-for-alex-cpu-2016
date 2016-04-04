#ifndef LLVM_LIB_TARGET_ALEX_ALEXREGISTERINFO_H
#define LLVM_LIB_TARGET_ALEX_ALEXREGISTERINFO_H

#include "llvm/Target/TargetRegisterInfo.h"

#define GET_REGINFO_HEADER
#include "AlexGenRegisterInfo.inc"

namespace llvm {
    class AlexSubtarget;
    struct AlexRegisterInfo : public AlexGenRegisterInfo {
    public:
        AlexRegisterInfo(const AlexSubtarget *subtarget);
    protected:
        const AlexSubtarget *subtarget;

        const MCPhysReg *getCalleeSavedRegs(const MachineFunction *MF) const override;
        const uint32_t *getCallPreservedMask(const MachineFunction &MF, CallingConv::ID CC) const override;
        BitVector getReservedRegs(const MachineFunction &MF) const override;

        const TargetRegisterClass *getPointerRegClass(const MachineFunction &MF, unsigned Kind) const override;

        void eliminateFrameIndex(MachineBasicBlock::iterator II,
                                 int SPAdj, unsigned FIOperandNum,
                                 RegScavenger *RS = nullptr) const override;

        unsigned getFrameRegister(const MachineFunction &MF) const override;

        bool canRealignStack(const MachineFunction &MF) const override {
            return false;
        }
    };

} // end namespace llvm

#endif