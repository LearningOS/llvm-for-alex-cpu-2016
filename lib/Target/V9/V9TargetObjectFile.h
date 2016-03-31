//===-- V9TargetObjectFile.h - V9 Object Info -------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SPARC_SPARCTARGETOBJECTFILE_H
#define LLVM_LIB_TARGET_SPARC_SPARCTARGETOBJECTFILE_H

#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"

namespace llvm {

class MCContext;
class TargetMachine;

class V9ELFTargetObjectFile : public TargetLoweringObjectFileELF {
public:
  V9ELFTargetObjectFile() :
    TargetLoweringObjectFileELF()
  {}

  const MCExpr *
  getTTypeGlobalReference(const GlobalValue *GV, unsigned Encoding,
                          Mangler &Mang, const TargetMachine &TM,
                          MachineModuleInfo *MMI,
                          MCStreamer &Streamer) const override;
};

} // end namespace llvm

#endif
