//===-- AlexMCCodeEmitter.h - Convert Alex Code to Machine Code -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the AlexMCCodeEmitter class.
//
//===----------------------------------------------------------------------===//
//

#ifndef LLVM_LIB_TARGET_ALEX_MCTARGETDESC_ALEXMCCODEEMITTER_H
#define LLVM_LIB_TARGET_ALEX_MCTARGETDESC_ALEXMCCODEEMITTER_H

//#include "AlexConfig.h"

#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/Support/DataTypes.h"

using namespace llvm;

namespace llvm {
    class MCContext;
    class MCExpr;
    class MCInst;
    class MCInstrInfo;
    class MCFixup;
    class MCOperand;
    class MCSubtargetInfo;
    class raw_ostream;

    class AlexMCCodeEmitter : public MCCodeEmitter {
        AlexMCCodeEmitter(const AlexMCCodeEmitter &) = delete;
        void operator=(const AlexMCCodeEmitter &) = delete;
        const MCInstrInfo &MCII;
        MCContext &Ctx;
        //bool IsLittleEndian;

    public:
        AlexMCCodeEmitter(const MCInstrInfo &mcii, MCContext &Ctx_)
                : MCII(mcii), Ctx(Ctx_) {}

        ~AlexMCCodeEmitter() override {}

        void EmitByte(unsigned char C, raw_ostream &OS) const;

        void EmitInstruction(uint64_t Val, unsigned Size, raw_ostream &OS) const;

        void encodeInstruction(const MCInst &MI, raw_ostream &OS,
                               SmallVectorImpl<MCFixup> &Fixups,
                               const MCSubtargetInfo &STI) const override;

        // getBinaryCodeForInstr - TableGen'erated function for getting the
        // binary encoding for an instruction.
        uint64_t getBinaryCodeForInstr(const MCInst &MI,
                                       SmallVectorImpl<MCFixup> &Fixups,
                                       const MCSubtargetInfo &STI) const;

        // getBranch16TargetOpValue - Return binary encoding of the branch
        // target operand, such as BEQ, BNE. If the machine operand
        // requires relocation, record the relocation and return zero.
        unsigned getBranch16TargetOpValue(const MCInst &MI, unsigned OpNo,
                                          SmallVectorImpl<MCFixup> &Fixups,
                                          const MCSubtargetInfo &STI) const;

        // getBranch24TargetOpValue - Return binary encoding of the branch
        // target operand, such as JMP #BB01, JEQ, JSUB. If the machine operand
        // requires relocation, record the relocation and return zero.
        unsigned getBranch24TargetOpValue(const MCInst &MI, unsigned OpNo,
                                          SmallVectorImpl<MCFixup> &Fixups,
                                          const MCSubtargetInfo &STI) const;

        // getJumpTargetOpValue - Return binary encoding of the jump
        // target operand, such as JSUB #function_addr.
        // If the machine operand requires relocation,
        // record the relocation and return zero.
        unsigned getJumpTargetOpValue(const MCInst &MI, unsigned OpNo,
                                      SmallVectorImpl<MCFixup> &Fixups,
                                      const MCSubtargetInfo &STI) const;

        // getMachineOpValue - Return binary encoding of operand. If the machin
        // operand requires relocation, record the relocation and return zero.
        unsigned getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                                   SmallVectorImpl<MCFixup> &Fixups,
                                   const MCSubtargetInfo &STI) const;

        unsigned getMemEncoding(const MCInst &MI, unsigned OpNo,
                                SmallVectorImpl<MCFixup> &Fixups,
                                const MCSubtargetInfo &STI) const;

        unsigned getExprOpValue(const MCExpr *Expr, SmallVectorImpl<MCFixup> &Fixups,
                                const MCSubtargetInfo &STI) const;
    }; // class AlexMCCodeEmitter
} // namespace llvm.

#endif