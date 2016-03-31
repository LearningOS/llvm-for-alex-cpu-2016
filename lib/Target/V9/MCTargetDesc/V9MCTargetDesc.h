//===-- V9MCTargetDesc.h - V9 Target Descriptions -----------*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_V9_MCTARGETDESC_V9MCTARGETDESC_H
#define LLVM_LIB_TARGET_V9_MCTARGETDESC_V9MCTARGETDESC_H

#include "V9Config.h"
#include "llvm/Support/DataTypes.h"

namespace llvm {
    class Target;
    class Triple;

    extern Target TheV9Target;
    extern Target TheV9elTarget;

} // End llvm namespace

// Defines symbolic names for V9 registers.  This defines a mapping from
// register name to register number.
#define GET_REGINFO_ENUM
#include "V9GenRegisterInfo.inc"

// Defines symbolic names for the V9 instructions.
#define GET_INSTRINFO_ENUM
#include "V9GenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "V9GenSubtargetInfo.inc"

#endif