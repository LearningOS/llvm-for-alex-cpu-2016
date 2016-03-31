//===-- V9MCTargetDesc.h - V9 Target Descriptions ---------*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_SPARC_MCTARGETDESC_SPARCMCTARGETDESC_H
#define LLVM_LIB_TARGET_SPARC_MCTARGETDESC_SPARCMCTARGETDESC_H

#include "llvm/Support/DataTypes.h"

namespace llvm {
class MCAsmBackend;
class MCCodeEmitter;
class MCContext;
class MCInstrInfo;
class MCObjectWriter;
class MCRegisterInfo;
class MCSubtargetInfo;
class Target;
class Triple;
class StringRef;
class raw_pwrite_stream;
class raw_ostream;

extern Target TheV9Target;
extern Target TheV9V9Target;
extern Target TheV9elTarget;

MCCodeEmitter *createV9MCCodeEmitter(const MCInstrInfo &MCII,
                                        const MCRegisterInfo &MRI,
                                        MCContext &Ctx);
MCAsmBackend *createV9AsmBackend(const Target &T, const MCRegisterInfo &MRI,
                                    const Triple &TT, StringRef CPU);
MCObjectWriter *createV9ELFObjectWriter(raw_pwrite_stream &OS, bool Is64Bit,
                                           bool IsLIttleEndian, uint8_t OSABI);
} // End llvm namespace

// Defines symbolic names for V9 registers.  This defines a mapping from
// register name to register number.
//
#define GET_REGINFO_ENUM
#include "V9GenRegisterInfo.inc"

// Defines symbolic names for the V9 instructions.
//
#define GET_INSTRINFO_ENUM
#include "V9GenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "V9GenSubtargetInfo.inc"

#endif
