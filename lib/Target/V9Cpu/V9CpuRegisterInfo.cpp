#include "V9CpuRegisterInfo.h"
#include "V9CpuInstrInfo.h"
#include "V9CpuFrameLowering.h"
#include "V9CpuSubtarget.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/Target/TargetInstrInfo.h"

using namespace llvm;
#define GET_REGINFO_TARGET_DESC
#include "V9CpuGenRegisterInfo.inc"

V9CpuRegisterInfo::V9CpuRegisterInfo(const V9CpuSubtarget &ST)
 : V9CpuGenRegisterInfo(V9Cpu::PC), Subtarget(ST) {}

const MCPhysReg*
V9CpuRegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
    return CSR_CalleeSavedRegs_SaveList;
}

const uint32_t *
V9CpuRegisterInfo::getCallPreservedMask(const MachineFunction &MF,
                                     CallingConv::ID CC) const {
    return CSR_CalleeSavedRegs_RegMask;
}

BitVector V9CpuRegisterInfo::getReservedRegs(const MachineFunction &MF) const {
    BitVector Reserved(getNumRegs());
    Reserved.set(V9Cpu::PC);
    Reserved.set(V9Cpu::FLAGS);
    return Reserved;
}

const TargetRegisterClass*
V9CpuRegisterInfo::getPointerRegClass(const MachineFunction &MF, unsigned Kind) const {
    return &V9Cpu::IntRegsRegClass;
}

void
V9CpuRegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                    int SPAdj, unsigned FIOperandNum,
                                    RegScavenger *RS) const {
}

unsigned V9CpuRegisterInfo::getFrameRegister(const MachineFunction &MF) const {
    return V9Cpu::FP;
}

// V9Cpu has no architectural need for stack realignment support,
// except that LLVM unfortunately currently implements overaligned
// stack objects by depending upon stack realignment support.
// If that ever changes, this can probably be deleted.
bool V9CpuRegisterInfo::canRealignStack(const MachineFunction &MF) const {
    return true;

    // V9Cpu always has a fixed frame pointer register, so don't need to
    // worry about needing to reserve it. [even if we don't have a frame
    // pointer for our frame, it still cannot be used for other things,
    // or register window traps will be SADNESS.]

    // If there's a reserved call frame, we can use SP to access locals.
    //if (getFrameLowering(MF)->hasReservedCallFrame(MF))
    //    return true;
}
