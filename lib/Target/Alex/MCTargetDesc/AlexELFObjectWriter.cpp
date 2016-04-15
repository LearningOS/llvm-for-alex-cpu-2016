//===-- AlexELFObjectWriter.cpp - Alex ELF Writer -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//


//#include "MCTargetDesc/AlexBaseInfo.h"
//#include "MCTargetDesc/AlexFixupKinds.h"
#include "MCTargetDesc/AlexMCTargetDesc.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCSection.h"
#include "llvm/MC/MCValue.h"
#include "llvm/Support/ErrorHandling.h"
#include <list>

using namespace llvm;

namespace {
    class AlexELFObjectWriter : public MCELFObjectTargetWriter {
    public:
        AlexELFObjectWriter(uint8_t OSABI);

        ~AlexELFObjectWriter() override;

        unsigned getRelocType(MCContext &Ctx, const MCValue &Target,
                                      const MCFixup &Fixup, bool IsPCRel) const override;
        bool needsRelocateWithSymbol(const MCSymbol &Sym,
                                     unsigned Type) const override;
    };
}

AlexELFObjectWriter::AlexELFObjectWriter(uint8_t OSABI)
        : MCELFObjectTargetWriter(/*_is64Bit=false*/ false, OSABI, ELF::EM_ALEX,
        /*HasRelocationAddend*/ false) {}

AlexELFObjectWriter::~AlexELFObjectWriter() {}

//@GetRelocType {
unsigned AlexELFObjectWriter::getRelocType(MCContext &Ctx, const MCValue &Target,
                                           const MCFixup &Fixup,
                                           bool IsPCRel) const {
    // determine the type of the relocation
    unsigned Type = (unsigned)ELF::R_ALEX_32;
    unsigned Kind = (unsigned)Fixup.getKind();
    return Type;

    switch (Kind) {
        default:
            llvm_unreachable("invalid fixup kind!");
        case FK_Data_4:
            Type = ELF::R_ALEX_32;
            break;
        /*case Alex::fixup_Alex_32:
            Type = ELF::R_ALEX_32;
            break;
        case Alex::fixup_Alex_GPREL16:
            Type = ELF::R_ALEX_GPREL16;
            break;
        case Alex::fixup_Alex_GOT_Global:
        case Alex::fixup_Alex_GOT_Local:
            Type = ELF::R_ALEX_GOT16;
            break;
        case Alex::fixup_Alex_HI16:
            Type = ELF::R_ALEX_HI16;
            break;
        case Alex::fixup_Alex_LO16:
            Type = ELF::R_ALEX_LO16;
            break;
        case Alex::fixup_Alex_GOT_HI16:
            Type = ELF::R_ALEX_GOT_HI16;
            break;
        case Alex::fixup_Alex_GOT_LO16:
            Type = ELF::R_ALEX_GOT_LO16;
            break;*/
    }

    return Type;
}
//@GetRelocType }

bool
AlexELFObjectWriter::needsRelocateWithSymbol(const MCSymbol &Sym,
                                             unsigned Type) const {
    // FIXME: This is extremelly conservative. This really needs to use a
    // whitelist with a clear explanation for why each realocation needs to
    // point to the symbol, not to the section.
   /* switch (Type) {
        default:
            return true;

        case ELF::R_ALEX_GOT16:
            // For Alex pic mode, I think it's OK to return true but I didn't confirm.
            //  llvm_unreachable("Should have been handled already");
            return true;

            // These relocations might be paired with another relocation. The pairing is
            // done by the static linker by matching the symbol. Since we only see one
            // relocation at a time, we have to force them to relocate with a symbol to
            // avoid ending up with a pair where one points to a section and another
            // points to a symbol.
        case ELF::R_ALEX_HI16:
        case ELF::R_ALEX_LO16:
            // R_ALEX_32 should be a relocation record, I don't know why Mips set it to 
            // false.
        case ELF::R_ALEX_32:
            return true;

        case ELF::R_ALEX_GPREL16:
            return false;
    }*/
    return true;
}

MCObjectWriter *llvm::createAlexELFObjectWriter(raw_pwrite_stream &OS,
                                                uint8_t OSABI,
                                                bool IsLittleEndian) {
    MCELFObjectTargetWriter *MOTW = new AlexELFObjectWriter(OSABI);
    return createELFObjectWriter(MOTW, OS, IsLittleEndian);
}