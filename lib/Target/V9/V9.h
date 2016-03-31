//===-- V9.h - Top-level interface for V9 representation ----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the entry points for global functions defined in
// the LLVM V9 back-end.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_V9_V9_H
#define LLVM_LIB_TARGET_V9_V9_H

#include "V9Config.h"
#include "MCTargetDesc/V9MCTargetDesc.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {
    class V9TargetMachine;
    class FunctionPass;

} // end namespace llvm;

#endif