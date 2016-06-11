#include "AlexRegisterInfo.h"
#include "AlexTargetMachine.h"
#include "AlexMachineFunction.h"
#include "AlexFrameLowering.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/Target/TargetInstrInfo.h"
using namespace llvm;

#define DEBUG_TYPE "v9cpu-register-info"

#define GET_REGINFO_TARGET_DESC
#include "AlexGenRegisterInfo.inc"

AlexRegisterInfo::AlexRegisterInfo(const AlexSubtarget *subtarget)
        :AlexGenRegisterInfo(Alex::LR /* RA */), subtarget(subtarget) {}

unsigned AlexRegisterInfo::getFrameRegister(const MachineFunction &MF) const {
    return Alex::FP;
    //return Alex::SP;
}

const MCPhysReg *AlexRegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
    return CSR_CalleeSavedRegs_SaveList;
}
const uint32_t *AlexRegisterInfo::getCallPreservedMask(const MachineFunction &MF, CallingConv::ID CC) const {
    return CSR_CalleeSavedRegs_RegMask;
}
BitVector AlexRegisterInfo::getReservedRegs(const MachineFunction &MF) const {
    BitVector Reserved(getNumRegs());
    Reserved.set(Alex::SP);
    Reserved.set(Alex::FP);
    Reserved.set(Alex::R0);
    Reserved.set(Alex::AT);
    Reserved.set(Alex::LR);
    Reserved.set(Alex::GP);
    return Reserved;
}

const TargetRegisterClass *AlexRegisterInfo::getPointerRegClass(const MachineFunction &MF, unsigned Kind) const {
    return &Alex::Int32RegsRegClass;
}


void AlexRegisterInfo::
eliminateFrameIndex(MachineBasicBlock::iterator II, int SPAdj,
                    unsigned FIOperandNum, RegScavenger *RS) const {
    MachineInstr &MI = *II;
    MachineFunction &MF = *MI.getParent()->getParent();
    MachineFrameInfo *MFI = MF.getFrameInfo();
    AlexFunctionInfo *AlexFI = MF.getInfo<AlexFunctionInfo>();

    unsigned i = 0;
    while (!MI.getOperand(i).isFI()) {
        ++i;
        assert(i < MI.getNumOperands() &&
               "Instr doesn't have FrameIndex operand!");
    }

    DEBUG(errs() << "\nFunction : " << MF.getFunction()->getName() << "\n";
                  errs() << "<--------->\n" << MI);

    int FrameIndex = MI.getOperand(i).getIndex();
    if (FrameIndex == -3) {
        FrameIndex = -3;
    }
    uint64_t stackSize = MF.getFrameInfo()->getStackSize();
    int64_t spOffset = MF.getFrameInfo()->getObjectOffset(FrameIndex);

    DEBUG(errs() << "FrameIndex : " << FrameIndex << "\n"
          << "spOffset   : " << spOffset << "\n"
          << "stackSize  : " << stackSize << "\n");

    const std::vector<CalleeSavedInfo> &CSI = MFI->getCalleeSavedInfo();
    //int MinCSFI = 0;
    //int MaxCSFI = -1;

    if (CSI.size()) {
        //MinCSFI = CSI[0].getFrameIdx();
        //MaxCSFI = CSI[CSI.size() - 1].getFrameIdx();
    }

    // The following stack frame objects are always referenced relative to $sp:
    //  1. Outgoing arguments.
    //  2. Pointer to dynamically allocated stack space.
    //  3. Locations for callee-saved registers.
    // Everything else is referenced relative to whatever register
    // getFrameRegister() returns.
    unsigned FrameReg;
//
//    int MinCSFI = 0;
//    int MaxCSFI = -1;
//    if (AlexFI->isOutArgFI(FrameIndex) || AlexFI->isDynAllocFI(FrameIndex) ||
//                (FrameIndex >= MinCSFI && FrameIndex <= MaxCSFI))
//        FrameReg = Alex::SP;
//    else
    FrameReg = Alex::FP;

    // Calculate final offset.
    // - There is no need to change the offset if the frame object is one of the
    //   following: an outgoing argument, pointer to a dynamically allocated
    //   stack space or a $gp restore location,
    // - If the frame object is any of the following, its offset must be adjusted
    //   by adding the size of the stack:
    //   incoming argument, callee-saved register location or local variable.
    int64_t Offset;
//    Offset = spOffset;

//    if (AlexFI->isOutArgFI(FrameIndex) || AlexFI->isDynAllocFI(FrameIndex))
//        Offset = spOffset + (int64_t)stackSize;
//    else
    Offset = spOffset;

    Offset    += MI.getOperand(i+1).getImm();

    DEBUG(errs() << "Offset     : " << Offset << "\n" << "<--------->\n");

    // If MI is not a debug value, make sure Offset fits in the 16-bit immediate
    // field.
    if (!MI.isDebugValue() && !isInt<16>(Offset)) {
        assert("(!MI.isDebugValue() && !isInt<16>(Offset))");
    }

    MI.getOperand(i).ChangeToRegister(FrameReg, false);
    MI.getOperand(i+1).ChangeToImmediate(Offset);


//
//    MachineInstr &MI = *II;
//    MachineFunction &MF = *MI.getParent()->getParent();
//    MachineFrameInfo *MFI = MF.getFrameInfo();
//    Cpu0FunctionInfo *Cpu0FI = MF.getInfo<Cpu0FunctionInfo>();
//
//    unsigned i = 0;
//    while (!MI.getOperand(i).isFI()) {
//        ++i;
//        assert(i < MI.getNumOperands() &&
//               "Instr doesn't have FrameIndex operand!");
//    }
//
//    DEBUG(errs() << "\nFunction : " << MF.getFunction()->getName() << "\n";
//                  errs() << "<--------->\n" << MI);
//
//    int FrameIndex = MI.getOperand(i).getIndex();
//    uint64_t stackSize = MF.getFrameInfo()->getStackSize();
//    int64_t spOffset = MF.getFrameInfo()->getObjectOffset(FrameIndex);
//
//    DEBUG(errs() << "FrameIndex : " << FrameIndex << "\n"
//          << "spOffset   : " << spOffset << "\n"
//          << "stackSize  : " << stackSize << "\n");
//
//    const std::vector<CalleeSavedInfo> &CSI = MFI->getCalleeSavedInfo();
//    int MinCSFI = 0;
//    int MaxCSFI = -1;
//
//    if (CSI.size()) {
//        MinCSFI = CSI[0].getFrameIdx();
//        MaxCSFI = CSI[CSI.size() - 1].getFrameIdx();
//    }
//
//    // The following stack frame objects are always referenced relative to $sp:
//    //  1. Outgoing arguments.
//    //  2. Pointer to dynamically allocated stack space.
//    //  3. Locations for callee-saved registers.
//    // Everything else is referenced relative to whatever register
//    // getFrameRegister() returns.
//    unsigned FrameReg;
//
//    if (Cpu0FI->isOutArgFI(FrameIndex) || Cpu0FI->isDynAllocFI(FrameIndex) ||
//        (FrameIndex >= MinCSFI && FrameIndex <= MaxCSFI))
//        FrameReg = Alex::SP;
//    else
//        FrameReg = getFrameRegister(MF);
//    // Calculate final offset.
//    // - There is no need to change the offset if the frame object is one of the
//    //   following: an outgoing argument, pointer to a dynamically allocated
//    //   stack space or a $gp restore location,
//    // - If the frame object is any of the following, its offset must be adjusted
//    //   by adding the size of the stack:
//    //   incoming argument, callee-saved register location or local variable.
//    int64_t Offset;
//
//    if (Cpu0FI->isOutArgFI(FrameIndex) || Cpu0FI->isGPFI(FrameIndex) ||
//      Cpu0FI->isDynAllocFI(FrameIndex))
//        Offset = spOffset;
//    else
//        Offset = spOffset + (int64_t)stackSize;
//
//    Offset    += MI.getOperand(i+1).getImm();
//
//    DEBUG(errs() << "Offset     : " << Offset << "\n" << "<--------->\n");
//
//    // If MI is not a debug value, make sure Offset fits in the 16-bit immediate
//    // field.
//    if (!MI.isDebugValue() && !isInt<16>(Offset)) {
//        assert("(!MI.isDebugValue() && !isInt<16>(Offset))");
//    }
//
//    MI.getOperand(i).ChangeToRegister(FrameReg, false);
//    MI.getOperand(i+1).ChangeToImmediate(Offset);
}