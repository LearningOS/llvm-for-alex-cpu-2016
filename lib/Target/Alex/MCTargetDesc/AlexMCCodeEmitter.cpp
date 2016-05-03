//===-- AlexMCCodeEmitter.cpp - Convert Alex Code to Machine Code ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the AlexMCCodeEmitter class.
//
//===----------------------------------------------------------------------===//
//

#include "AlexMCCodeEmitter.h"

//#include "MCTargetDesc/AlexBaseInfo.h"
//#include "MCTargetDesc/AlexFixupKinds.h"
//#include "MCTargetDesc/AlexMCExpr.h"
#include "MCTargetDesc/AlexMCTargetDesc.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/raw_ostream.h"

#define DEBUG_TYPE "mccodeemitter"

#define GET_INSTRMAP_INFO
#include "AlexGenInstrInfo.inc"
#include "AlexMCExpr.h"
#include "AlexFixupKinds.h"

#undef GET_INSTRMAP_INFO

namespace llvm {
    MCCodeEmitter *createAlexMCCodeEmitterEL(const MCInstrInfo &MCII,
                                             const MCRegisterInfo &MRI,
                                             MCContext &Context) {
        return new AlexMCCodeEmitter(MCII, Context);
    }

} // End of namespace llvm

void AlexMCCodeEmitter::EmitByte(unsigned char C, raw_ostream &OS) const {
    OS << (char)C;
}

void AlexMCCodeEmitter::EmitInstruction(uint64_t Val, unsigned Size, raw_ostream &OS) const {
    // Output the instruction encoding in little endian byte order.
    for (unsigned i = 0; i < Size; ++i) {
        unsigned Shift = i;
        EmitByte((Val >> (Shift*8)) & 0xff, OS);
    }
}

/// encodeInstruction - Emit the instruction.
/// Size the instruction (currently only 4 bytes)
void AlexMCCodeEmitter::
encodeInstruction(const MCInst &MI, raw_ostream &OS,
                  SmallVectorImpl<MCFixup> &Fixups,
                  const MCSubtargetInfo &STI) const
{
    uint32_t Binary = getBinaryCodeForInstr(MI, Fixups, STI);

    // Check for unimplemented opcodes.
    // Unfortunately in ALEX both NOT and SLL will come in with Binary == 0
    // so we have to special check for them.
    unsigned Opcode = MI.getOpcode();
    //if ((Opcode != Alex::NOP) && (Opcode != Alex::SHL) && !Binary)
    //    llvm_unreachable("unimplemented opcode in encodeInstruction()");

    const MCInstrDesc &Desc = MCII.get(MI.getOpcode());
    uint64_t TSFlags = Desc.TSFlags;

    // Pseudo instructions don't get encoded and shouldn't be here
    // in the first place!
    //if ((TSFlags & AlexII::FormMask) == AlexII::Pseudo)
   //     llvm_unreachable("Pseudo opcode found in encodeInstruction()");

    // For now all instructions are 4 bytes
    int Size = 4; // FIXME: Have Desc.getSize() return the correct value!

    EmitInstruction(Binary, Size, OS);
}

//@CH8_1 {
/// getBranch16TargetOpValue - Return binary encoding of the branch
/// target operand. If the machine operand requires relocation,
/// record the relocation and return zero.
unsigned AlexMCCodeEmitter::
getBranch16TargetOpValue(const MCInst &MI, unsigned OpNo,
                         SmallVectorImpl<MCFixup> &Fixups,
                         const MCSubtargetInfo &STI) const {
    const MCOperand &MO = MI.getOperand(OpNo);

    // If the destination is an immediate, we have nothing to do.
    if (MO.isImm()) return MO.getImm();
    assert(MO.isExpr() && "getBranch16TargetOpValue expects only expressions");

    const MCExpr *Expr = MO.getExpr();
    Fixups.push_back(MCFixup::create(0, Expr,
                                     MCFixupKind(Alex::fixup_Alex_PC16)));
    return 0;
}

/// getJumpTargetOpValue - Return binary encoding of the jump
/// target operand, such as JSUB. 
/// If the machine operand requires relocation,
/// record the relocation and return zero.
//@getJumpTargetOpValue {
unsigned AlexMCCodeEmitter::
getJumpTargetOpValue(const MCInst &MI, unsigned OpNo,
                     SmallVectorImpl<MCFixup> &Fixups,
                     const MCSubtargetInfo &STI) const {
    return 0;
}
//@CH8_1 }

//@getExprOpValue {
unsigned AlexMCCodeEmitter::
getExprOpValue(const MCExpr *Expr,SmallVectorImpl<MCFixup> &Fixups,
               const MCSubtargetInfo &STI) const {
//@getExprOpValue body {
    MCExpr::ExprKind Kind = Expr->getKind();
    if (Kind == MCExpr::Constant) {
        return cast<MCConstantExpr>(Expr)->getValue();
    }

    if (Kind == MCExpr::Binary) {
        unsigned Res = getExprOpValue(cast<MCBinaryExpr>(Expr)->getLHS(), Fixups, STI);
        Res += getExprOpValue(cast<MCBinaryExpr>(Expr)->getRHS(), Fixups, STI);
        return Res;
    }

    if (Kind == MCExpr::Target) {
        const AlexMCExpr *AlexExpr = cast<AlexMCExpr>(Expr);

        Alex::Fixups FixupKind = Alex::Fixups(0);
        switch (AlexExpr->getKind()) {
            default: llvm_unreachable("Unsupported fixup kind for target expression!");
            case AlexMCExpr::VK_Alex_HI:
                FixupKind = Alex::fixup_Alex_HI16;
                break;
            case AlexMCExpr::VK_Alex_LO:
                FixupKind = Alex::fixup_Alex_LO16;
                break;
        }
        Fixups.push_back(MCFixup::create(0, AlexExpr, MCFixupKind(FixupKind)));
        return 0;
    }

    if (Kind == MCExpr::SymbolRef) {
        Alex::Fixups FixupKind = Alex::Fixups(0);

        //@switch {
        auto a = cast<MCSymbolRefExpr>(Expr)->getKind();
        switch(a) {
        //    @switch }
        //case MCSymbolRefExpr::VK_Alex_GPREL:
        //    FixupKind = Alex::fixup_Alex_GPREL16;
        //    break;
//#if CH >= CH9_1 //2
//        case MCSymbolRefExpr::VK_Alex_GOT_CALL:
//            FixupKind = Alex::fixup_Alex_CALL16;
//            break;
//#endif
//        case MCSymbolRefExpr::VK_Alex_GOT16:
//            FixupKind = Alex::fixup_Alex_GOT_Global;
//            break;
//        case MCSymbolRefExpr::VK_Alex_GOT:
//            FixupKind = Alex::fixup_Alex_GOT_Local;
//            break;
        case MCSymbolRefExpr::VK_Alex_ABS_HI:
            FixupKind = Alex::fixup_Alex_HI16;
            break;
        case MCSymbolRefExpr::VK_Alex_ABS_LO:
            FixupKind = Alex::fixup_Alex_LO16;
            break;
//#if CH >= CH12_1
//        case MCSymbolRefExpr::VK_Alex_TLSGD:
//            FixupKind = Alex::fixup_Alex_TLSGD;
//            break;
//        case MCSymbolRefExpr::VK_Alex_TLSLDM:
//            FixupKind = Alex::fixup_Alex_TLSLDM;
//            break;
//        case MCSymbolRefExpr::VK_Alex_DTP_HI:
//            FixupKind = Alex::fixup_Alex_DTP_HI;
//            break;
//        case MCSymbolRefExpr::VK_Alex_DTP_LO:
//            FixupKind = Alex::fixup_Alex_DTP_LO;
//            break;
//        case MCSymbolRefExpr::VK_Alex_GOTTPREL:
//            FixupKind = Alex::fixup_Alex_GOTTPREL;
//            break;
//        case MCSymbolRefExpr::VK_Alex_TP_HI:
//            FixupKind = Alex::fixup_Alex_TP_HI;
//            break;
//        case MCSymbolRefExpr::VK_Alex_TP_LO:
//            FixupKind = Alex::fixup_Alex_TP_LO;
//            break;
//#endif
//        case MCSymbolRefExpr::VK_Alex_GOT_HI16:
//            FixupKind = Alex::fixup_Alex_GOT_HI16;
//            break;
//        case MCSymbolRefExpr::VK_Alex_GOT_LO16:
//            FixupKind = Alex::fixup_Alex_GOT_LO16;
//            break;
        default:
            break;
        } // switch

        Fixups.push_back(MCFixup::create(0, Expr, MCFixupKind(FixupKind)));
        return 0;
    }
    
    // All of the information is in the fixup.
    return 0;
}

/// getMachineOpValue - Return binary encoding of operand. If the machine
/// operand requires relocation, record the relocation and return zero.
unsigned AlexMCCodeEmitter::
getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                  SmallVectorImpl<MCFixup> &Fixups,
                  const MCSubtargetInfo &STI) const {
    if (MO.isReg()) {
        unsigned Reg = MO.getReg();
        unsigned RegNo = Ctx.getRegisterInfo()->getEncodingValue(Reg);
        return RegNo;
    } else if (MO.isImm()) {
        return static_cast<unsigned>(MO.getImm());
    } else if (MO.isFPImm()) {
        return static_cast<unsigned>(APFloat(MO.getFPImm())
                .bitcastToAPInt().getHiBits(32).getLimitedValue());
    }
    // MO must be an Expr.
    assert(MO.isExpr());
    return getExprOpValue(MO.getExpr(),Fixups, STI);
}

/// getMemEncoding - Return binary encoding of memory related operand.
/// If the offset operand requires relocation, record the relocation.
unsigned
AlexMCCodeEmitter::getMemEncoding(const MCInst &MI, unsigned OpNo,
                                  SmallVectorImpl<MCFixup> &Fixups,
                                  const MCSubtargetInfo &STI) const {
    // Base register is encoded in bits 19-16, offset is encoded in bits 15-0.
    assert(MI.getOperand(OpNo).isReg());
    unsigned RegBits = getMachineOpValue(MI, MI.getOperand(OpNo), Fixups, STI) << 16;
    unsigned OffBits = getMachineOpValue(MI, MI.getOperand(OpNo+1), Fixups, STI);

    return (OffBits & 0xFFFF) | RegBits;
}


#include "AlexGenMCCodeEmitter.inc"