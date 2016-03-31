//===-- V9TargetMachine.cpp - Define TargetMachine for V9 -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Implements the info about V9 target spec.
//
//===----------------------------------------------------------------------===//

#include "V9TargetMachine.h"
#include "V9.h"

#include "llvm/IR/LegacyPassManager.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;

#define DEBUG_TYPE "V9"

extern "C" void LLVMInitializeV9Target() {
}