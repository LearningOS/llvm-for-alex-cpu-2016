#ifndef LLVM_LIB_TARGET_V9CPU_V9CPUFRAMELOWERING_H
#define LLVM_LIB_TARGET_V9CPU_V9CPUFRAMELOWERING_H

#include "llvm/Target/TargetFrameLowering.h"

namespace llvm {
    class V9CpuSubtarget;
    class V9CpuFrameLowering : public TargetFrameLowering {
    protected:
        const V9CpuSubtarget &STI;
    public:
        explicit V9CpuFrameLowering(const V9CpuSubtarget &sti ,unsigned Alignment)
                : TargetFrameLowering(StackGrowsDown, Alignment, 0, Alignment), STI(sti) {
        }

        static const V9CpuFrameLowering *create(const V9CpuSubtarget &ST);

        bool hasFP(const MachineFunction &MF) const override;
    };

    const V9CpuFrameLowering *createV9CpuSEFrameLowering(const V9CpuSubtarget &ST);

} // End llvm namespace

#endif