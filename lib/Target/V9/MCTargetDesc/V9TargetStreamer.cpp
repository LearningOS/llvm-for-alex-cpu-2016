//===-- V9TargetStreamer.cpp - V9 Target Streamer Methods -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides V9 specific target streamer methods.
//
//===----------------------------------------------------------------------===//

#include "V9TargetStreamer.h"
#include "InstPrinter/V9InstPrinter.h"
#include "llvm/Support/FormattedStream.h"

using namespace llvm;

// pin vtable to this file
V9TargetStreamer::V9TargetStreamer(MCStreamer &S) : MCTargetStreamer(S) {}

void V9TargetStreamer::anchor() {}

V9TargetAsmStreamer::V9TargetAsmStreamer(MCStreamer &S,
                                               formatted_raw_ostream &OS)
    : V9TargetStreamer(S), OS(OS) {}

void V9TargetAsmStreamer::emitV9RegisterIgnore(unsigned reg) {
  OS << "\t.register "
     << "%" << StringRef(V9InstPrinter::getRegisterName(reg)).lower()
     << ", #ignore\n";
}

void V9TargetAsmStreamer::emitV9RegisterScratch(unsigned reg) {
  OS << "\t.register "
     << "%" << StringRef(V9InstPrinter::getRegisterName(reg)).lower()
     << ", #scratch\n";
}

V9TargetELFStreamer::V9TargetELFStreamer(MCStreamer &S)
    : V9TargetStreamer(S) {}

MCELFStreamer &V9TargetELFStreamer::getStreamer() {
  return static_cast<MCELFStreamer &>(Streamer);
}
