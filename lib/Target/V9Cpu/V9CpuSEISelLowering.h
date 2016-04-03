//===-- V9CpuISEISelLowering.h - V9CpuISE DAG Lowering Interface ----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Subclass of V9CpuITargetLowering specialized for cpu032/64.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_V9CPU_V9CPUSEISELLOWERING_H
#define LLVM_LIB_TARGET_V9CPU_V9CPUSEISELLOWERING_H


#include "V9CpuISelLowering.h"
#include "V9CpuRegisterInfo.h"

namespace llvm {
  class V9CpuSETargetLowering : public V9CpuTargetLowering  {
  public:
    explicit V9CpuSETargetLowering(const V9CpuTargetMachine &TM,
                                  const V9CpuSubtarget &STI);

    SDValue LowerOperation(SDValue Op, SelectionDAG &DAG) const override;
  private:
  };
  const V9CpuTargetLowering *
    createV9CpuSETargetLowering(const V9CpuTargetMachine &TM, const V9CpuSubtarget &STI);
}

#endif // V9CpuISEISELLOWERING_H