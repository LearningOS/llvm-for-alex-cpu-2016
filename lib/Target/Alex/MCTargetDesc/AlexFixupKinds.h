//===-- AlexFixupKinds.h - Alex Specific Fixup Entries ----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_ALEX_MCTARGETDESC_ALEXFIXUPKINDS_H
#define LLVM_LIB_TARGET_ALEX_MCTARGETDESC_ALEXFIXUPKINDS_H

//#include "AlexConfig.h"

#include "llvm/MC/MCFixup.h"

namespace llvm {
    namespace Alex {
        // Although most of the current fixup types reflect a unique relocation
        // one can have multiple fixup types for a given relocation and thus need
        // to be uniquely named.
        //
        // This table *must* be in the save order of
        // MCFixupKindInfo Infos[Alex::NumTargetFixupKinds]
        // in AlexAsmBackend.cpp.
        //@Fixups {
        enum Fixups {
            //@ Pure upper 32 bit fixup resulting in - R_ALEX_32.
                    fixup_Alex_Invalid = FirstTargetFixupKind,
                    fixup_Alex_32,

            // Pure upper 16 bit fixup resulting in - R_ALEX_HI16.
                    fixup_Alex_HI16,

            // Pure lower 16 bit fixup resulting in - R_ALEX_LO16.
                    fixup_Alex_LO16,

            // 16 bit fixup for GP offest resulting in - R_ALEX_GPREL16.
                    fixup_Alex_GPREL16,

            // Global symbol fixup resulting in - R_ALEX_GOT16.
                    fixup_Alex_GOT_Global,

            // Local symbol fixup resulting in - R_ALEX_GOT16.
                    fixup_Alex_GOT_Local,

            // PC relative branch fixup resulting in - R_ALEX_PC16.
            // cpu0 PC16, e.g. beq
                    fixup_Alex_PC16,

            // PC relative branch fixup resulting in - R_ALEX_PC24.
            // cpu0 PC24, e.g. jeq, jmp
                    fixup_Alex_PC24,


            // resulting in - R_ALEX_CALL16.
                    fixup_Alex_CALL16,

            // resulting in - R_ALEX_TLS_GD.
                    fixup_Alex_TLSGD,

            // resulting in - R_ALEX_TLS_GOTTPREL.
                    fixup_Alex_GOTTPREL,

            // resulting in - R_ALEX_TLS_TPREL_HI16.
                    fixup_Alex_TP_HI,

            // resulting in - R_ALEX_TLS_TPREL_LO16.
                    fixup_Alex_TP_LO,

            // resulting in - R_ALEX_TLS_LDM.
                    fixup_Alex_TLSLDM,

            // resulting in - R_ALEX_TLS_DTP_HI16.
                    fixup_Alex_DTP_HI,

            // resulting in - R_ALEX_TLS_DTP_LO16.
                    fixup_Alex_DTP_LO,

            // resulting in - R_ALEX_GOT_HI16
                    fixup_Alex_GOT_HI16,

            // resulting in - R_ALEX_GOT_LO16
                    fixup_Alex_GOT_LO16,

            // Marker
                    LastTargetFixupKind,
            NumTargetFixupKinds = LastTargetFixupKind - FirstTargetFixupKind
        };
        //@Fixups }
    } // namespace Alex
} // namespace llvm


#endif // LLVM_ALEX_ALEXFIXUPKINDS_H