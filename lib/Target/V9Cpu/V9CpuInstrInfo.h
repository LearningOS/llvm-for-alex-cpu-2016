#ifndef LLVM_LIB_TARGET_V9CPU_V9CPUINSTRINFO_H
#define LLVM_LIB_TARGET_V9CPU_V9CPUINSTRINFO_H

#include "V9Cpu.h"
#include "V9CpuRegisterInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/Target/TargetInstrInfo.h"

#define GET_INSTRINFO_HEADER
#include "V9CpuGenInstrInfo.inc"

namespace llvm {

    class V9CpuInstrInfo : public V9CpuGenInstrInfo {
        virtual void anchor();
    protected:
        const V9CpuSubtarget &Subtarget;
    public:
        explicit V9CpuInstrInfo(const V9CpuSubtarget &STI);

        static const V9CpuInstrInfo *create(V9CpuSubtarget &STI);

        /// getRegisterInfo - TargetInstrInfo is a superset of MRegister info.  As
        /// such, whenever a client has an instance of instruction info, it should
        /// always be able to get register info as well (through this method).
        ///
        virtual const V9CpuRegisterInfo &getRegisterInfo() const = 0;
        /// Return the number of bytes of code the specified instruction may be.
        unsigned GetInstSizeInBytes(const MachineInstr *MI) const {
            return 4;
        }
        virtual void adjustStackPtr(unsigned SP, int64_t Amount,
                                    MachineBasicBlock &MBB,
                                    MachineBasicBlock::iterator I) const = 0;
        virtual MachineInstr* emitFrameIndexDebugValue(MachineFunction &MF,
                                                         int FrameIx, uint64_t Offset,
                                                         const MDNode *MDPtr,
                                                         DebugLoc DL) const;
    protected:
        MachineMemOperand *GetMemOperand(MachineBasicBlock &MBB, int FI, unsigned int Flag) const;
    };
    const V9CpuInstrInfo *createV9CpuSEInstrInfo(const V9CpuSubtarget &STI);
}

#endif