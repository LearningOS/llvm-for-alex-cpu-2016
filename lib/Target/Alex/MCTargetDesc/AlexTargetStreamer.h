//===-- AlexTargetStreamer.h - Alex Target Streamer ------------*- C++ -*--===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_ALEX_ALEXTARGETSTREAMER_H
#define LLVM_LIB_TARGET_ALEX_ALEXTARGETSTREAMER_H


#include "llvm/MC/MCELFStreamer.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCStreamer.h"

namespace llvm {

    class AlexTargetStreamer : public MCTargetStreamer {
    public:
        AlexTargetStreamer(MCStreamer &S);
    };

// This part is for ascii assembly output
    class AlexTargetAsmStreamer : public AlexTargetStreamer {
        formatted_raw_ostream &OS;

    public:
        AlexTargetAsmStreamer(MCStreamer &S, formatted_raw_ostream &OS);
    };

}

#endif