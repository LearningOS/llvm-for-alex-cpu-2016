//===-- V9MCTargetDesc.cpp - V9 Target Descriptions -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides V9 specific target descriptions.
//
//===----------------------------------------------------------------------===//

#include "V9MCTargetDesc.h"
#include "InstPrinter/V9InstPrinter.h"
#include "V9MCAsmInfo.h"
#include "V9TargetStreamer.h"
#include "llvm/MC/MCCodeGenInfo.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

#define GET_INSTRINFO_MC_DESC
#include "V9GenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "V9GenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "V9GenRegisterInfo.inc"

static MCAsmInfo *createV9MCAsmInfo(const MCRegisterInfo &MRI,
                                       const Triple &TT) {
  MCAsmInfo *MAI = new V9ELFMCAsmInfo(TT);
  unsigned Reg = MRI.getDwarfRegNum(SP::O6, true);
  MCCFIInstruction Inst = MCCFIInstruction::createDefCfa(nullptr, Reg, 0);
  MAI->addInitialFrameState(Inst);
  return MAI;
}

static MCAsmInfo *createV9V9MCAsmInfo(const MCRegisterInfo &MRI,
                                         const Triple &TT) {
  MCAsmInfo *MAI = new V9ELFMCAsmInfo(TT);
  unsigned Reg = MRI.getDwarfRegNum(SP::O6, true);
  MCCFIInstruction Inst = MCCFIInstruction::createDefCfa(nullptr, Reg, 2047);
  MAI->addInitialFrameState(Inst);
  return MAI;
}

static MCInstrInfo *createV9MCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitV9MCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createV9MCRegisterInfo(const Triple &TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitV9MCRegisterInfo(X, SP::O7);
  return X;
}

static MCSubtargetInfo *
createV9MCSubtargetInfo(const Triple &TT, StringRef CPU, StringRef FS) {
  if (CPU.empty())
    CPU = (TT.getArch() == Triple::sparcv9) ? "v9" : "v8";
  return createV9MCSubtargetInfoImpl(TT, CPU, FS);
}

// Code models. Some only make sense for 64-bit code.
//
// SunCC  Reloc   CodeModel  Constraints
// abs32  Static  Small      text+data+bss linked below 2^32 bytes
// abs44  Static  Medium     text+data+bss linked below 2^44 bytes
// abs64  Static  Large      text smaller than 2^31 bytes
// pic13  PIC_    Small      GOT < 2^13 bytes
// pic32  PIC_    Medium     GOT < 2^32 bytes
//
// All code models require that the text segment is smaller than 2GB.

static MCCodeGenInfo *createV9MCCodeGenInfo(const Triple &TT,
                                               Reloc::Model RM,
                                               CodeModel::Model CM,
                                               CodeGenOpt::Level OL) {
  MCCodeGenInfo *X = new MCCodeGenInfo();

  // The default 32-bit code model is abs32/pic32 and the default 32-bit
  // code model for JIT is abs32.
  switch (CM) {
  default: break;
  case CodeModel::Default:
  case CodeModel::JITDefault: CM = CodeModel::Small; break;
  }

  X->initMCCodeGenInfo(RM, CM, OL);
  return X;
}

static MCCodeGenInfo *createV9V9MCCodeGenInfo(const Triple &TT,
                                                 Reloc::Model RM,
                                                 CodeModel::Model CM,
                                                 CodeGenOpt::Level OL) {
  MCCodeGenInfo *X = new MCCodeGenInfo();

  // The default 64-bit code model is abs44/pic32 and the default 64-bit
  // code model for JIT is abs64.
  switch (CM) {
  default:  break;
  case CodeModel::Default:
    CM = RM == Reloc::PIC_ ? CodeModel::Small : CodeModel::Medium;
    break;
  case CodeModel::JITDefault:
    CM = CodeModel::Large;
    break;
  }

  X->initMCCodeGenInfo(RM, CM, OL);
  return X;
}

static MCTargetStreamer *
createObjectTargetStreamer(MCStreamer &S, const MCSubtargetInfo &STI) {
  return new V9TargetELFStreamer(S);
}

static MCTargetStreamer *createTargetAsmStreamer(MCStreamer &S,
                                                 formatted_raw_ostream &OS,
                                                 MCInstPrinter *InstPrint,
                                                 bool isVerboseAsm) {
  return new V9TargetAsmStreamer(S, OS);
}

static MCInstPrinter *createV9MCInstPrinter(const Triple &T,
                                               unsigned SyntaxVariant,
                                               const MCAsmInfo &MAI,
                                               const MCInstrInfo &MII,
                                               const MCRegisterInfo &MRI) {
  return new V9InstPrinter(MAI, MII, MRI);
}

extern "C" void LLVMInitializeV9TargetMC() {
  // Register the MC asm info.
  RegisterMCAsmInfoFn X(TheV9Target, createV9MCAsmInfo);
  RegisterMCAsmInfoFn Y(TheV9V9Target, createV9V9MCAsmInfo);
  RegisterMCAsmInfoFn Z(TheV9elTarget, createV9MCAsmInfo);

  for (Target *T : {&TheV9Target, &TheV9V9Target, &TheV9elTarget}) {
    // Register the MC instruction info.
    TargetRegistry::RegisterMCInstrInfo(*T, createV9MCInstrInfo);

    // Register the MC register info.
    TargetRegistry::RegisterMCRegInfo(*T, createV9MCRegisterInfo);

    // Register the MC subtarget info.
    TargetRegistry::RegisterMCSubtargetInfo(*T, createV9MCSubtargetInfo);

    // Register the MC Code Emitter.
    TargetRegistry::RegisterMCCodeEmitter(*T, createV9MCCodeEmitter);

    // Register the asm backend.
    TargetRegistry::RegisterMCAsmBackend(*T, createV9AsmBackend);

    // Register the object target streamer.
    TargetRegistry::RegisterObjectTargetStreamer(*T,
                                                 createObjectTargetStreamer);

    // Register the asm streamer.
    TargetRegistry::RegisterAsmTargetStreamer(*T, createTargetAsmStreamer);

    // Register the MCInstPrinter
    TargetRegistry::RegisterMCInstPrinter(*T, createV9MCInstPrinter);
  }

  // Register the MC codegen info.
  TargetRegistry::RegisterMCCodeGenInfo(TheV9Target,
                                        createV9MCCodeGenInfo);
  TargetRegistry::RegisterMCCodeGenInfo(TheV9V9Target,
                                        createV9V9MCCodeGenInfo);
  TargetRegistry::RegisterMCCodeGenInfo(TheV9elTarget,
                                        createV9MCCodeGenInfo);
}
