//===-- V9TargetInfo.cpp - V9 Target Implementation -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "V9.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;

Target llvm::TheV9Target;
Target llvm::TheV9V9Target;
Target llvm::TheV9elTarget;

extern "C" void LLVMInitializeV9TargetInfo() {
  RegisterTarget<Triple::sparc, /*HasJIT=*/true> X(TheV9Target, "sparc",
                                                   "V9");
  RegisterTarget<Triple::sparcv9, /*HasJIT=*/true> Y(TheV9V9Target,
                                                     "sparcv9", "V9 V9");
  RegisterTarget<Triple::sparcel, /*HasJIT=*/true> Z(TheV9elTarget,
                                                     "sparcel", "V9 LE");
}
