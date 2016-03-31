//===-- V9TargetInfo.cpp - V9 Target Implementation -------------------===//
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

extern "C" void LLVMInitializeV9TargetInfo() {
    RegisterTarget<Triple::V9, /*HasJIT=*/false> X(TheV9Target, "v9", "V9");
}
