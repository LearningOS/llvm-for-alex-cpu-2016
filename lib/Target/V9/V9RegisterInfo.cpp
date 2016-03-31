//===-- V9RegisterInfo.cpp - SPARC Register Information ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the SPARC implementation of the TargetRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#include "V9RegisterInfo.h"
#include "V9.h"
#include "V9MachineFunctionInfo.h"
#include "V9Subtarget.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Target/TargetInstrInfo.h"

using namespace llvm;

#define GET_REGINFO_TARGET_DESC
#include "V9GenRegisterInfo.inc"

static cl::opt<bool>
ReserveAppRegisters("sparc-reserve-app-registers", cl::Hidden, cl::init(false),
                    cl::desc("Reserve application registers (%g2-%g4)"));

V9RegisterInfo::V9RegisterInfo() : V9GenRegisterInfo(SP::O7) {}

static const MCPhysReg CSR_SaveList[] = { 0 };
const MCPhysReg*
V9RegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  return CSR_SaveList;
}

static const uint32_t CSR_RegMask[] = { 0 };
const uint32_t *
V9RegisterInfo::getCallPreservedMask(const MachineFunction &MF,
                                        CallingConv::ID CC) const {
  return CSR_RegMask;
}

const uint32_t*
V9RegisterInfo::getRTCallPreservedMask(CallingConv::ID CC) const {
  return RTCSR_RegMask;
}

BitVector V9RegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());
  const V9Subtarget &Subtarget = MF.getSubtarget<V9Subtarget>();

  //Reserved.set(SP::I0);
  //Reserved.set(SP::I1);
  //Reserved.set(SP::I2);
  //Reserved.set(SP::I3);
  //Reserved.set(SP::I4);
  //Reserved.set(SP::I5);
//  Reserved.set(SP::I6);
//  Reserved.set(SP::I7);
//  Reserved.set(SP::I0_I1);
//  Reserved.set(SP::I2_I3);
//  Reserved.set(SP::I4_I5);
//  Reserved.set(SP::I6_I7);

  //Reserved.set(SP::G0);
  //Reserved.set(SP::G1);
  //Reserved.set(SP::G2);
  //Reserved.set(SP::G3);
  //Reserved.set(SP::G4);
  //Reserved.set(SP::G5);
  //Reserved.set(SP::G6);
  //Reserved.set(SP::G7);

//  Reserved.set(SP::L0);
//  Reserved.set(SP::L1);
//  Reserved.set(SP::L2);
//  Reserved.set(SP::L3);
//  Reserved.set(SP::L4);
//  Reserved.set(SP::L5);
//  Reserved.set(SP::L6);
//  Reserved.set(SP::L7);
//  Reserved.set(SP::L0_L1);
//  Reserved.set(SP::L2_L3);
//  Reserved.set(SP::L4_L5);
//  Reserved.set(SP::L6_L7);

  Reserved.set(SP::O0);
  Reserved.set(SP::O1);
  Reserved.set(SP::O2);
  Reserved.set(SP::O3);
  Reserved.set(SP::O4);
  Reserved.set(SP::O5);
  Reserved.set(SP::O6);
  Reserved.set(SP::O7);
  Reserved.set(SP::O0_O1);
  Reserved.set(SP::O2_O3);
  Reserved.set(SP::O4_O5);
  Reserved.set(SP::O6_O7);
  return Reserved;
}

const TargetRegisterClass*
V9RegisterInfo::getPointerRegClass(const MachineFunction &MF,
                                      unsigned Kind) const {
  const V9Subtarget &Subtarget = MF.getSubtarget<V9Subtarget>();
  return Subtarget.is64Bit() ? &SP::I64RegsRegClass : &SP::IntRegsRegClass;
}

static void replaceFI(MachineFunction &MF,
                      MachineBasicBlock::iterator II,
                      MachineInstr &MI,
                      DebugLoc dl,
                      unsigned FIOperandNum, int Offset,
                      unsigned FramePtr)
{
  // Replace frame index with a frame pointer reference.
  if (Offset >= -4096 && Offset <= 4095) {
    // If the offset is small enough to fit in the immediate field, directly
    // encode it.
    MI.getOperand(FIOperandNum).ChangeToRegister(FramePtr, false);
    MI.getOperand(FIOperandNum + 1).ChangeToImmediate(Offset);
    return;
  }

  const TargetInstrInfo &TII = *MF.getSubtarget().getInstrInfo();

  // FIXME: it would be better to scavenge a register here instead of
  // reserving G1 all of the time.
  if (Offset >= 0) {
    // Emit nonnegaive immediates with sethi + or.
    // sethi %hi(Offset), %g1
    // add %g1, %fp, %g1
    // Insert G1+%lo(offset) into the user.
    BuildMI(*MI.getParent(), II, dl, TII.get(SP::SETHIi), SP::G1)
      .addImm(HI22(Offset));


    // Emit G1 = G1 + I6
    BuildMI(*MI.getParent(), II, dl, TII.get(SP::ADDrr), SP::G1).addReg(SP::G1)
      .addReg(FramePtr);
    // Insert: G1+%lo(offset) into the user.
    MI.getOperand(FIOperandNum).ChangeToRegister(SP::G1, false);
    MI.getOperand(FIOperandNum + 1).ChangeToImmediate(LO10(Offset));
    return;
  }

  // Emit Negative numbers with sethi + xor
  // sethi %hix(Offset), %g1
  // xor  %g1, %lox(offset), %g1
  // add %g1, %fp, %g1
  // Insert: G1 + 0 into the user.
  BuildMI(*MI.getParent(), II, dl, TII.get(SP::SETHIi), SP::G1)
    .addImm(HIX22(Offset));
  BuildMI(*MI.getParent(), II, dl, TII.get(SP::XORri), SP::G1)
    .addReg(SP::G1).addImm(LOX10(Offset));

  BuildMI(*MI.getParent(), II, dl, TII.get(SP::ADDrr), SP::G1).addReg(SP::G1)
    .addReg(FramePtr);
  // Insert: G1+%lo(offset) into the user.
  MI.getOperand(FIOperandNum).ChangeToRegister(SP::G1, false);
  MI.getOperand(FIOperandNum + 1).ChangeToImmediate(0);
}


void
V9RegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                       int SPAdj, unsigned FIOperandNum,
                                       RegScavenger *RS) const {
  assert(SPAdj == 0 && "Unexpected");

  MachineInstr &MI = *II;
  DebugLoc dl = MI.getDebugLoc();
  int FrameIndex = MI.getOperand(FIOperandNum).getIndex();
  MachineFunction &MF = *MI.getParent()->getParent();
  const V9Subtarget &Subtarget = MF.getSubtarget<V9Subtarget>();
  const V9FrameLowering *TFI = getFrameLowering(MF);

  unsigned FrameReg;
  int Offset;
  Offset = TFI->getFrameIndexReference(MF, FrameIndex, FrameReg);

  Offset += MI.getOperand(FIOperandNum + 1).getImm();

  if (!Subtarget.isV9() || !Subtarget.hasHardQuad()) {
    if (MI.getOpcode() == SP::STQFri) {
      const TargetInstrInfo &TII = *Subtarget.getInstrInfo();
      unsigned SrcReg = MI.getOperand(2).getReg();
      unsigned SrcEvenReg = getSubReg(SrcReg, SP::sub_even64);
      unsigned SrcOddReg  = getSubReg(SrcReg, SP::sub_odd64);
      MachineInstr *StMI =
        BuildMI(*MI.getParent(), II, dl, TII.get(SP::STDFri))
        .addReg(FrameReg).addImm(0).addReg(SrcEvenReg);
      replaceFI(MF, II, *StMI, dl, 0, Offset, FrameReg);
      MI.setDesc(TII.get(SP::STDFri));
      MI.getOperand(2).setReg(SrcOddReg);
      Offset += 8;
    } else if (MI.getOpcode() == SP::LDQFri) {
      const TargetInstrInfo &TII = *Subtarget.getInstrInfo();
      unsigned DestReg     = MI.getOperand(0).getReg();
      unsigned DestEvenReg = getSubReg(DestReg, SP::sub_even64);
      unsigned DestOddReg  = getSubReg(DestReg, SP::sub_odd64);
      MachineInstr *StMI =
        BuildMI(*MI.getParent(), II, dl, TII.get(SP::LDDFri), DestEvenReg)
        .addReg(FrameReg).addImm(0);
      replaceFI(MF, II, *StMI, dl, 1, Offset, FrameReg);

      MI.setDesc(TII.get(SP::LDDFri));
      MI.getOperand(0).setReg(DestOddReg);
      Offset += 8;
    }
  }

  replaceFI(MF, II, MI, dl, FIOperandNum, Offset, FrameReg);

}

unsigned V9RegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  return SP::A6;
}

// V9 has no architectural need for stack realignment support,
// except that LLVM unfortunately currently implements overaligned
// stack objects by depending upon stack realignment support.
// If that ever changes, this can probably be deleted.
bool V9RegisterInfo::canRealignStack(const MachineFunction &MF) const {
  if (!TargetRegisterInfo::canRealignStack(MF))
    return false;

  // V9 always has a fixed frame pointer register, so don't need to
  // worry about needing to reserve it. [even if we don't have a frame
  // pointer for our frame, it still cannot be used for other things,
  // or register window traps will be SADNESS.]

  // If there's a reserved call frame, we can use SP to access locals.
  if (getFrameLowering(MF)->hasReservedCallFrame(MF))
    return true;

  // Otherwise, we'd need a base pointer, but those aren't implemented
  // for SPARC at the moment.

  return false;
}
