#include "V9CpuRegisterInfo.h"
#include "V9CpuInstrInfo.h"
#include "V9CpuFrameLowering.h"
#include "V9CpuSubtarget.h"
#include "V9CpuMachineFunction.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/Target/TargetInstrInfo.h"

#define DEBUG_TYPE "v9cpu-register-info"

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
    Reserved.set(V9Cpu::ZERO);
    return Reserved;
}

const TargetRegisterClass*
V9CpuRegisterInfo::getPointerRegClass(const MachineFunction &MF, unsigned Kind) const {
    return &V9Cpu::IntRegsRegClass;
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

//- If no eliminateFrameIndex(), it will hang on run. 
// pure virtual method
// FrameIndex represent objects inside a abstract stack.
// We must replace FrameIndex with an stack/frame pointer
// direct reference.
void V9CpuRegisterInfo::
eliminateFrameIndex(MachineBasicBlock::iterator II, int SPAdj,
                    unsigned FIOperandNum, RegScavenger *RS) const {
    MachineInstr &MI = *II;
    MachineFunction &MF = *MI.getParent()->getParent();
    MachineFrameInfo *MFI = MF.getFrameInfo();
    V9CpuFunctionInfo *V9CpuFI = MF.getInfo<V9CpuFunctionInfo>();

    unsigned i = 0;
    while (!MI.getOperand(i).isFI()) {
        ++i;
        assert(i < MI.getNumOperands() &&
               "Instr doesn't have FrameIndex operand!");
    }

    DEBUG(errs() << "\nFunction : " << MF.getFunction()->getName() << "\n";
                  errs() << "<--------->\n" << MI);

    int FrameIndex = MI.getOperand(i).getIndex();
    uint64_t stackSize = MF.getFrameInfo()->getStackSize();
    int64_t spOffset = MF.getFrameInfo()->getObjectOffset(FrameIndex);

    DEBUG(errs() << "FrameIndex : " << FrameIndex << "\n"
          << "spOffset   : " << spOffset << "\n"
          << "stackSize  : " << stackSize << "\n");

    const std::vector<CalleeSavedInfo> &CSI = MFI->getCalleeSavedInfo();
    int MinCSFI = 0;
    int MaxCSFI = -1;

    if (CSI.size()) {
        MinCSFI = CSI[0].getFrameIdx();
        MaxCSFI = CSI[CSI.size() - 1].getFrameIdx();
    }

    // The following stack frame objects are always referenced relative to $sp:
    //  1. Outgoing arguments.
    //  2. Pointer to dynamically allocated stack space.
    //  3. Locations for callee-saved registers.
    // Everything else is referenced relative to whatever register
    // getFrameRegister() returns.
    unsigned FrameReg;

    FrameReg = V9Cpu::SP;

    // Calculate final offset.
    // - There is no need to change the offset if the frame object is one of the
    //   following: an outgoing argument, pointer to a dynamically allocated
    //   stack space or a $gp restore location,
    // - If the frame object is any of the following, its offset must be adjusted
    //   by adding the size of the stack:
    //   incoming argument, callee-saved register location or local variable.
    int64_t Offset;
    Offset = spOffset + (int64_t)stackSize;

    Offset    += MI.getOperand(i+1).getImm();

    DEBUG(errs() << "Offset     : " << Offset << "\n" << "<--------->\n");

    // If MI is not a debug value, make sure Offset fits in the 16-bit immediate
    // field.
    if (!MI.isDebugValue() && !isInt<16>(Offset)) {
        assert("(!MI.isDebugValue() && !isInt<16>(Offset))");
    }

    MI.getOperand(i).ChangeToRegister(FrameReg, false);
    MI.getOperand(i+1).ChangeToImmediate(Offset);
}