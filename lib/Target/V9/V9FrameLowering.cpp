//===-- V9FrameLowering.cpp - V9 Frame Information ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the V9 implementation of TargetFrameLowering class.
//
//===----------------------------------------------------------------------===//

#include "V9FrameLowering.h"
#include "V9InstrInfo.h"
#include "V9MachineFunctionInfo.h"
#include "V9Subtarget.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Target/TargetOptions.h"

using namespace llvm;

constexpr uint32_t V9_STACK_ALIGNMENT = 8;

static cl::opt<bool>
DisableLeafProc("disable-sparc-leaf-proc",
                cl::init(false),
                cl::desc("Disable V9 leaf procedure optimization."),
                cl::Hidden);

V9FrameLowering::V9FrameLowering(const V9Subtarget &ST)
    : TargetFrameLowering(TargetFrameLowering::StackGrowsDown,
                          V9_STACK_ALIGNMENT, 0, V9_STACK_ALIGNMENT, /* TODO: stack real*/true) {}

void V9FrameLowering::emitSPAdjustment(MachineFunction &MF,
                                          MachineBasicBlock &MBB,
                                          MachineBasicBlock::iterator MBBI,
                                          int NumBytes,
                                          unsigned ADDrr,
                                          unsigned ADDri) const {

  DebugLoc dl;
  const V9InstrInfo &TII =
      *static_cast<const V9InstrInfo *>(MF.getSubtarget().getInstrInfo());

  if (NumBytes >= -4096 && NumBytes < 4096) {
    BuildMI(MBB, MBBI, dl, TII.get(ADDri), SP::O6)
      .addReg(SP::O6).addImm(NumBytes);
    return;
  }

  // Emit this the hard way.  This clobbers G1 which we always know is
  // available here.
  if (NumBytes >= 0) {
    // Emit nonnegative numbers with sethi + or.
    // sethi %hi(NumBytes), %g1
    // or %g1, %lo(NumBytes), %g1
    // add %sp, %g1, %sp
    BuildMI(MBB, MBBI, dl, TII.get(SP::SETHIi), SP::G1)
      .addImm(HI22(NumBytes));
    BuildMI(MBB, MBBI, dl, TII.get(SP::ORri), SP::G1)
      .addReg(SP::G1).addImm(LO10(NumBytes));
    BuildMI(MBB, MBBI, dl, TII.get(ADDrr), SP::O6)
      .addReg(SP::O6).addReg(SP::G1);
    return ;
  }

  // Emit negative numbers with sethi + xor.
  // sethi %hix(NumBytes), %g1
  // xor %g1, %lox(NumBytes), %g1
  // add %sp, %g1, %sp
  BuildMI(MBB, MBBI, dl, TII.get(SP::SETHIi), SP::G1)
    .addImm(HIX22(NumBytes));
  BuildMI(MBB, MBBI, dl, TII.get(SP::XORri), SP::G1)
    .addReg(SP::G1).addImm(LOX10(NumBytes));
  BuildMI(MBB, MBBI, dl, TII.get(ADDrr), SP::O6)
    .addReg(SP::O6).addReg(SP::G1);
}

void V9FrameLowering::emitPrologue(MachineFunction &MF,
                                      MachineBasicBlock &MBB) const {
  V9MachineFunctionInfo *FuncInfo = MF.getInfo<V9MachineFunctionInfo>();

  assert(&MF.front() == &MBB && "Shrink-wrapping not yet supported");
  MachineFrameInfo *MFI = MF.getFrameInfo();
  const V9InstrInfo &TII =
      *static_cast<const V9InstrInfo *>(MF.getSubtarget().getInstrInfo());
  const V9RegisterInfo &RegInfo =
      *static_cast<const V9RegisterInfo *>(MF.getSubtarget().getRegisterInfo());
  MachineBasicBlock::iterator MBBI = MBB.begin();
  // Debug location must be unknown since the first debug location is used
  // to determine the end of the prologue.
  DebugLoc dl;
  bool NeedsStackRealignment = RegInfo.needsStackRealignment(MF);

  // FIXME: unfortunately, returning false from canRealignStack
  // actually just causes needsStackRealignment to return false,
  // rather than reporting an error, as would be sensible. This is
  // poor, but fixing that bogosity is going to be a large project.
  // For now, just see if it's lied, and report an error here.
  if (!NeedsStackRealignment && MFI->getMaxAlignment() > getStackAlignment())
    report_fatal_error("Function \"" + Twine(MF.getName()) + "\" required "
                       "stack re-alignment, but LLVM couldn't handle it "
                       "(probably because it has a dynamic alloca).");

  // Get the number of bytes to allocate from the FrameInfo
  int NumBytes = (int) MFI->getStackSize();

  unsigned SAVEri = SP::SAVEri;
  unsigned SAVErr = SP::SAVErr;
  if (FuncInfo->isLeafProc()) {
    if (NumBytes == 0)
      return;
    SAVEri = SP::ADDri;
    SAVErr = SP::ADDrr;
  }

  // The SPARC ABI is a bit odd in that it requires a reserved 92-byte
  // (128 in v9) area in the user's stack, starting at %sp. Thus, the
  // first part of the stack that can actually be used is located at
  // %sp + 92.
  //
  // We therefore need to add that offset to the total stack size
  // after all the stack objects are placed by
  // PrologEpilogInserter calculateFrameObjectOffsets. However, since the stack needs to be
  // aligned *after* the extra size is added, we need to disable
  // calculateFrameObjectOffsets's built-in stack alignment, by having
  // targetHandlesStackFrameRounding return true.


  // Add the extra call frame stack size, if needed. (This is the same
  // code as in PrologEpilogInserter, but also gets disabled by
  // targetHandlesStackFrameRounding)
  if (MFI->adjustsStack() && hasReservedCallFrame(MF))
    NumBytes += MFI->getMaxCallFrameSize();

  // Adds the SPARC subtarget-specific spill area to the stack
  // size. Also ensures target-required alignment.
  NumBytes = MF.getSubtarget<V9Subtarget>().getAdjustedFrameSize(NumBytes);

  // Finally, ensure that the size is sufficiently aligned for the
  // data on the stack.
  if (MFI->getMaxAlignment() > 0) {
    NumBytes = alignTo(NumBytes, MFI->getMaxAlignment());
  }

  // Update stack size with corrected value.
  MFI->setStackSize(NumBytes);

  emitSPAdjustment(MF, MBB, MBBI, -NumBytes, SAVErr, SAVEri);

  MachineModuleInfo &MMI = MF.getMMI();
  unsigned regFP = RegInfo.getDwarfRegNum(SP::A6, true);

  // Emit ".cfi_def_cfa_register 30".
  unsigned CFIIndex =
      MMI.addFrameInst(MCCFIInstruction::createDefCfaRegister(nullptr, regFP));
  BuildMI(MBB, MBBI, dl, TII.get(TargetOpcode::CFI_INSTRUCTION))
      .addCFIIndex(CFIIndex);

  // Emit ".cfi_window_save".
  CFIIndex = MMI.addFrameInst(MCCFIInstruction::createWindowSave(nullptr));
  BuildMI(MBB, MBBI, dl, TII.get(TargetOpcode::CFI_INSTRUCTION))
      .addCFIIndex(CFIIndex);

  unsigned regInRA = RegInfo.getDwarfRegNum(SP::A7, true);
  unsigned regOutRA = RegInfo.getDwarfRegNum(SP::O7, true);
  // Emit ".cfi_register 15, 31".
  CFIIndex = MMI.addFrameInst(
      MCCFIInstruction::createRegister(nullptr, regOutRA, regInRA));
  BuildMI(MBB, MBBI, dl, TII.get(TargetOpcode::CFI_INSTRUCTION))
      .addCFIIndex(CFIIndex);

  if (NeedsStackRealignment) {
    // andn %o6, MaxAlign-1, %o6
    int MaxAlign = MFI->getMaxAlignment();
    BuildMI(MBB, MBBI, dl, TII.get(SP::ANDNri), SP::O6).addReg(SP::O6).addImm(MaxAlign - 1);
  }
}

void V9FrameLowering::
eliminateCallFramePseudoInstr(MachineFunction &MF, MachineBasicBlock &MBB,
                              MachineBasicBlock::iterator I) const {
  if (!hasReservedCallFrame(MF)) {
    MachineInstr &MI = *I;
    int Size = MI.getOperand(0).getImm();
    if (MI.getOpcode() == SP::ADJCALLSTACKDOWN)
      Size = -Size;

    if (Size)
      emitSPAdjustment(MF, MBB, I, Size, SP::ADDrr, SP::ADDri);
  }
  MBB.erase(I);
}


void V9FrameLowering::emitEpilogue(MachineFunction &MF,
                                  MachineBasicBlock &MBB) const {
  V9MachineFunctionInfo *FuncInfo = MF.getInfo<V9MachineFunctionInfo>();
  MachineBasicBlock::iterator MBBI = MBB.getLastNonDebugInstr();
  const V9InstrInfo &TII =
      *static_cast<const V9InstrInfo *>(MF.getSubtarget().getInstrInfo());
  DebugLoc dl = MBBI->getDebugLoc();
  assert(MBBI->getOpcode() == SP::RETL &&
         "Can only put epilog before 'retl' instruction!");
  if (!FuncInfo->isLeafProc()) {
    BuildMI(MBB, MBBI, dl, TII.get(SP::RESTORErr), SP::G0).addReg(SP::G0)
      .addReg(SP::G0);
    return;
  }
  MachineFrameInfo *MFI = MF.getFrameInfo();

  int NumBytes = (int) MFI->getStackSize();
  if (NumBytes == 0)
    return;

  emitSPAdjustment(MF, MBB, MBBI, NumBytes, SP::ADDrr, SP::ADDri);
}

bool V9FrameLowering::hasReservedCallFrame(const MachineFunction &MF) const {
  // Reserve call frame if there are no variable sized objects on the stack.
  return !MF.getFrameInfo()->hasVarSizedObjects();
}

// hasFP - Return true if the specified function should have a dedicated frame
// pointer register.  This is true if the function has variable sized allocas or
// if frame pointer elimination is disabled.
bool V9FrameLowering::hasFP(const MachineFunction &MF) const {
  const TargetRegisterInfo *RegInfo = MF.getSubtarget().getRegisterInfo();

  const MachineFrameInfo *MFI = MF.getFrameInfo();
  return MF.getTarget().Options.DisableFramePointerElim(MF) ||
      RegInfo->needsStackRealignment(MF) ||
      MFI->hasVarSizedObjects() ||
      MFI->isFrameAddressTaken();
}


int V9FrameLowering::getFrameIndexReference(const MachineFunction &MF, int FI,
                                               unsigned &FrameReg) const {
  const V9Subtarget &Subtarget = MF.getSubtarget<V9Subtarget>();
  const MachineFrameInfo *MFI = MF.getFrameInfo();
  const V9RegisterInfo *RegInfo = Subtarget.getRegisterInfo();
  const V9MachineFunctionInfo *FuncInfo = MF.getInfo<V9MachineFunctionInfo>();
  bool isFixed = MFI->isFixedObjectIndex(FI);

  // Addressable stack objects are accessed using neg. offsets from
  // %fp, or positive offsets from %sp.
  bool UseFP;

  // V9 uses FP-based references in general, even when "hasFP" is
  // false. That function is rather a misnomer, because %fp is
  // actually always available, unless isLeafProc.
  if (FuncInfo->isLeafProc()) {
    // If there's a leaf proc, all offsets need to be %sp-based,
    // because we haven't caused %fp to actually point to our frame.
    UseFP = false;
  } else if (isFixed) {
    // Otherwise, argument access should always use %fp.
    UseFP = true;
  } else if (RegInfo->needsStackRealignment(MF)) {
    // If there is dynamic stack realignment, all local object
    // references need to be via %sp, to take account of the
    // re-alignment.
    UseFP = false;
  } else {
    // Finally, default to using %fp.
    UseFP = true;
  }

  int64_t FrameOffset = MF.getFrameInfo()->getObjectOffset(FI) +
      Subtarget.getStackPointerBias();

  if (UseFP) {
    FrameReg = RegInfo->getFrameRegister(MF);
    return FrameOffset;
  } else {
    FrameReg = SP::O6; // %sp
    return FrameOffset + MF.getFrameInfo()->getStackSize();
  }
}

static bool LLVM_ATTRIBUTE_UNUSED verifyLeafProcRegUse(MachineRegisterInfo *MRI)
{

  for (unsigned reg = SP::A0; reg <= SP::A7; ++reg)
    if (!MRI->reg_nodbg_empty(reg))
      return false;

  return true;
}

bool V9FrameLowering::isLeafProc(MachineFunction &MF) const
{

  MachineRegisterInfo &MRI = MF.getRegInfo();
  MachineFrameInfo    *MFI = MF.getFrameInfo();

  return !(MFI->hasCalls()                 // has calls
           || !MRI.reg_nodbg_empty(SP::O6) // %SP is used
           || hasFP(MF));                  // need %FP
}

void V9FrameLowering::remapRegsForLeafProc(MachineFunction &MF) const {
  MachineRegisterInfo &MRI = MF.getRegInfo();
  // Remap %i[0-7] to %o[0-7].
  for (unsigned reg = SP::A0; reg <= SP::A7; ++reg) {
    if (MRI.reg_nodbg_empty(reg))
      continue;

    unsigned mapped_reg = reg - SP::A0 + SP::O0;
    assert(MRI.reg_nodbg_empty(mapped_reg));

    // Replace I register with O register.
    MRI.replaceRegWith(reg, mapped_reg);

    // Also replace register pair super-registers.
//    if ((reg - SP::A0) % 2 == 0) {
//      unsigned preg = (reg - SP::A0) / 2 + SP::I0_I1;
//      unsigned mapped_preg = preg - SP::I0_I1 + SP::O0_O1;
//      MRI.replaceRegWith(preg, mapped_preg);
//    }
  }

  // Rewrite MBB's Live-ins.
//  for (MachineFunction::iterator MBB = MF.begin(), E = MF.end();
//       MBB != E; ++MBB) {
//    for (unsigned reg = SP::I0_I1; reg <= SP::I6_I7; ++reg) {
//      if (!MBB->isLiveIn(reg))
//        continue;
//      MBB->removeLiveIn(reg);
//      MBB->addLiveIn(reg - SP::I0_I1 + SP::O0_O1);
//    }
//    for (unsigned reg = SP::I0; reg <= SP::I7; ++reg) {
//      if (!MBB->isLiveIn(reg))
//        continue;
//      MBB->removeLiveIn(reg);
//      MBB->addLiveIn(reg - SP::I0 + SP::O0);
//    }
//  }

  assert(verifyLeafProcRegUse(&MRI));
#ifdef XDEBUG
  MF.verify(0, "After LeafProc Remapping");
#endif
}

void V9FrameLowering::determineCalleeSaves(MachineFunction &MF,
                                              BitVector &SavedRegs,
                                              RegScavenger *RS) const {
  TargetFrameLowering::determineCalleeSaves(MF, SavedRegs, RS);
  if (!DisableLeafProc && isLeafProc(MF)) {
    V9MachineFunctionInfo *MFI = MF.getInfo<V9MachineFunctionInfo>();
    MFI->setLeafProc(true);

    remapRegsForLeafProc(MF);
  }

}
