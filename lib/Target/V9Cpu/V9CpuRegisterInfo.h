
#ifndef LLVM_LIB_TARGET_V9CPU_V9CPUREGISTERINFO_H
#define LLVM_LIB_TARGET_V9CPU_V9CPUREGISTERINFO_H

#include "llvm/Target/TargetRegisterInfo.h"

#define GET_REGINFO_HEADER
#include "V9CpuGenRegisterInfo.inc"

namespace llvm {
    struct V9CpuRegisterInfo : public V9CpuGenRegisterInfo {
        V9CpuRegisterInfo();
        const MCPhysReg *getCalleeSavedRegs(const MachineFunction *MF) const override;
        const uint32_t *getCallPreservedMask(const MachineFunction &MF, CallingConv::ID CC) const override;


        BitVector getReservedRegs(const MachineFunction &MF) const override;

        const TargetRegisterClass *getPointerRegClass(const MachineFunction &MF, unsigned Kind) const override;

        void eliminateFrameIndex(MachineBasicBlock::iterator II,
                                 int SPAdj, unsigned FIOperandNum,
                                 RegScavenger *RS = nullptr) const override;

        unsigned getFrameRegister(const MachineFunction &MF) const override;

        bool canRealignStack(const MachineFunction &MF) const override;
    };

} // end namespace llvm

#endif
