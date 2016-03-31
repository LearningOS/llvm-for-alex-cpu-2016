//===-- V9TargetStreamer.h - V9 Target Streamer ----------*- C++ -*--===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SPARC_SPARCTARGETSTREAMER_H
#define LLVM_LIB_TARGET_SPARC_SPARCTARGETSTREAMER_H

#include "llvm/MC/MCELFStreamer.h"
#include "llvm/MC/MCStreamer.h"

namespace llvm {
class V9TargetStreamer : public MCTargetStreamer {
  virtual void anchor();

public:
  V9TargetStreamer(MCStreamer &S);
  /// Emit ".register <reg>, #ignore".
  virtual void emitV9RegisterIgnore(unsigned reg) = 0;
  /// Emit ".register <reg>, #scratch".
  virtual void emitV9RegisterScratch(unsigned reg) = 0;
};

// This part is for ascii assembly output
class V9TargetAsmStreamer : public V9TargetStreamer {
  formatted_raw_ostream &OS;

public:
  V9TargetAsmStreamer(MCStreamer &S, formatted_raw_ostream &OS);
  void emitV9RegisterIgnore(unsigned reg) override;
  void emitV9RegisterScratch(unsigned reg) override;

};

// This part is for ELF object output
class V9TargetELFStreamer : public V9TargetStreamer {
public:
  V9TargetELFStreamer(MCStreamer &S);
  MCELFStreamer &getStreamer();
  void emitV9RegisterIgnore(unsigned reg) override {}
  void emitV9RegisterScratch(unsigned reg) override {}
};
} // end namespace llvm

#endif
