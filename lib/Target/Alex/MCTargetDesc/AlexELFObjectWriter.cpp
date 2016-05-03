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
#include "AlexFixupKinds.h"
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
    unsigned Type = (unsigned)ELF::R_ALEX_NONE;
    unsigned Kind = (unsigned)Fixup.getKind();

    switch (Kind) {
        default:
            llvm_unreachable("invalid fixup kind!");
        case FK_Data_4:
            Type = ELF::R_ALEX_32;
            break;
        case Alex::fixup_Alex_32:
            Type = ELF::R_ALEX_32;
            break;
        case Alex::fixup_Alex_HI16:
            Type = ELF::R_ALEX_HI16;
            break;
        case Alex::fixup_Alex_LO16:
            Type = ELF::R_ALEX_LO16;
            break;
        case Alex::fixup_Alex_PC16:
            Type = ELF::R_ALEX_PC16;
            break;
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
    switch (Type) {
        default:
            return true;
        case ELF::R_ALEX_HI16:
        case ELF::R_ALEX_LO16:
        case ELF::R_ALEX_32:
            return true;
    }
}

MCObjectWriter *llvm::createAlexELFObjectWriter(raw_pwrite_stream &OS,
                                                uint8_t OSABI) {
    MCELFObjectTargetWriter *MOTW = new AlexELFObjectWriter(OSABI);
    return createELFObjectWriter(MOTW, OS, true);
}