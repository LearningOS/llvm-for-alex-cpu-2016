#ifndef LLVM_LIB_TARGET_V9CPU_V9CPUSEINSTRINFO_H
#define LLVM_LIB_TARGET_V9CPU_V9CPUSEINSTRINFO_H


#include "V9CpuInstrInfo.h"
#include "V9CpuMachineFunction.h"
#include "V9CpuSERegisterInfo.h"

namespace llvm {

    class V9CpuSEInstrInfo : public V9CpuInstrInfo {
        const V9CpuSERegisterInfo RI;

    public:
        explicit V9CpuSEInstrInfo(const V9CpuSubtarget &STI);
        const V9CpuRegisterInfo &getRegisterInfo() const override;
        bool expandPostRAPseudo(MachineBasicBlock::iterator MI) const override;
        /// Adjust SP by Amount bytes.
        void adjustStackPtr(unsigned SP, int64_t Amount, MachineBasicBlock &MBB,
                            MachineBasicBlock::iterator I) const override;

        /// Emit a series of instructions to load an immediate. If NewImm is a
        /// non-NULL parameter, the last instruction is not emitted, but instead
        /// its immediate operand is returned in NewImm.
        unsigned loadImmediate(int64_t Imm, MachineBasicBlock &MBB,
                               MachineBasicBlock::iterator II, DebugLoc DL,
                               unsigned *NewImm) const;

    private:
        void expandRetLR(MachineBasicBlock &MBB, MachineBasicBlock::iterator I) const;

        void storeRegToStackSlot(MachineBasicBlock &MBB, MachineBasicBlock::iterator I, unsigned int SrcReg, bool isKill,
                             int FI, const TargetRegisterClass *RC, const TargetRegisterInfo *TRI) const override;

        void loadRegFromStackSlot(MachineBasicBlock &MBB, MachineBasicBlock::iterator I, unsigned int DestReg, int FI,
                              const TargetRegisterClass *RC, const TargetRegisterInfo *TRI) const override;
    };
}

#endif