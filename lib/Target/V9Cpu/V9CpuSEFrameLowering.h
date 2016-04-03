#ifndef LLVM_LIB_TARGET_V9CPU_V9CPUSEFRAMELOWERING_H
#define LLVM_LIB_TARGET_V9CPU_V9CPUSEFRAMELOWERING_H

#include "V9CpuFrameLowering.h"
#include "V9CpuSubtarget.h"

namespace llvm {

    class V9CpuSEFrameLowering : public V9CpuFrameLowering {
    public:
        explicit V9CpuSEFrameLowering(const V9CpuSubtarget &STI);

        /// emitProlog/emitEpilog - These methods insert prolog and epilog code into
        /// the function.
        void emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const override;
        void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const override;

        bool hasReservedCallFrame(const MachineFunction &MF) const;

        void determineCalleeSaves(MachineFunction &MF, BitVector &SavedRegs, RegScavenger *RS) const;
    };

} // End llvm namespace

#endif