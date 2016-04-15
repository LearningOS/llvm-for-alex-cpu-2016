//===-- AlexTargetStreamer.cpp - Alex Target Streamer Methods -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides Alex specific target streamer methods.
//
//===----------------------------------------------------------------------===//

#include "InstPrinter/AlexInstPrinter.h"
#include "AlexMCTargetDesc.h"
#include "AlexTargetObjectFile.h"
#include "AlexTargetStreamer.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCSectionELF.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCSymbolELF.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ELF.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FormattedStream.h"

#if CH >= CH5_1

using namespace llvm;

AlexTargetStreamer::AlexTargetStreamer(MCStreamer &S)
        : MCTargetStreamer(S) {
}

AlexTargetAsmStreamer::AlexTargetAsmStreamer(MCStreamer &S,
                                             formatted_raw_ostream &OS)
        : AlexTargetStreamer(S), OS(OS) {}

#endif // #if CH >= CH5_1