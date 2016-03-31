//===-- V9AsmBackend.cpp - V9 Assembler Backend ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "llvm/MC/MCAsmBackend.h"
#include "MCTargetDesc/V9FixupKinds.h"
#include "MCTargetDesc/V9MCTargetDesc.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCFixupKindInfo.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCValue.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

static unsigned adjustFixupValue(unsigned Kind, uint64_t Value) {
  switch (Kind) {
  default:
    llvm_unreachable("Unknown fixup kind!");
  case FK_Data_1:
  case FK_Data_2:
  case FK_Data_4:
  case FK_Data_8:
    return Value;

  case V9::fixup_sparc_wplt30:
  case V9::fixup_sparc_call30:
    return (Value >> 2) & 0x3fffffff;

  case V9::fixup_sparc_br22:
    return (Value >> 2) & 0x3fffff;

  case V9::fixup_sparc_br19:
    return (Value >> 2) & 0x7ffff;

  case V9::fixup_sparc_br16_2:
    return (Value >> 2) & 0xc000;

  case V9::fixup_sparc_br16_14:
    return (Value >> 2) & 0x3fff;

  case V9::fixup_sparc_pc22:
  case V9::fixup_sparc_got22:
  case V9::fixup_sparc_tls_gd_hi22:
  case V9::fixup_sparc_tls_ldm_hi22:
  case V9::fixup_sparc_tls_ie_hi22:
  case V9::fixup_sparc_hi22:
    return (Value >> 10) & 0x3fffff;

  case V9::fixup_sparc_pc10:
  case V9::fixup_sparc_got10:
  case V9::fixup_sparc_tls_gd_lo10:
  case V9::fixup_sparc_tls_ldm_lo10:
  case V9::fixup_sparc_tls_ie_lo10:
  case V9::fixup_sparc_lo10:
    return Value & 0x3ff;

  case V9::fixup_sparc_tls_ldo_hix22:
  case V9::fixup_sparc_tls_le_hix22:
    return (~Value >> 10) & 0x3fffff;

  case V9::fixup_sparc_tls_ldo_lox10:
  case V9::fixup_sparc_tls_le_lox10:
    return (~(~Value & 0x3ff)) & 0x1fff;

  case V9::fixup_sparc_h44:
    return (Value >> 22) & 0x3fffff;

  case V9::fixup_sparc_m44:
    return (Value >> 12) & 0x3ff;

  case V9::fixup_sparc_l44:
    return Value & 0xfff;

  case V9::fixup_sparc_hh:
    return (Value >> 42) & 0x3fffff;

  case V9::fixup_sparc_hm:
    return (Value >> 32) & 0x3ff;

  case V9::fixup_sparc_tls_gd_add:
  case V9::fixup_sparc_tls_gd_call:
  case V9::fixup_sparc_tls_ldm_add:
  case V9::fixup_sparc_tls_ldm_call:
  case V9::fixup_sparc_tls_ldo_add:
  case V9::fixup_sparc_tls_ie_ld:
  case V9::fixup_sparc_tls_ie_ldx:
  case V9::fixup_sparc_tls_ie_add:
    return 0;
  }
}

namespace {
  class V9AsmBackend : public MCAsmBackend {
  protected:
    const Target &TheTarget;
    bool IsLittleEndian;
    bool Is64Bit;

  public:
    V9AsmBackend(const Target &T)
        : MCAsmBackend(), TheTarget(T),
          IsLittleEndian(StringRef(TheTarget.getName()) == "sparcel"),
          Is64Bit(StringRef(TheTarget.getName()) == "sparcv9") {}

    unsigned getNumFixupKinds() const override {
      return V9::NumTargetFixupKinds;
    }

    const MCFixupKindInfo &getFixupKindInfo(MCFixupKind Kind) const override {
      const static MCFixupKindInfo InfosBE[V9::NumTargetFixupKinds] = {
        // name                    offset bits  flags
        { "fixup_sparc_call30",     2,     30,  MCFixupKindInfo::FKF_IsPCRel },
        { "fixup_sparc_br22",      10,     22,  MCFixupKindInfo::FKF_IsPCRel },
        { "fixup_sparc_br19",      13,     19,  MCFixupKindInfo::FKF_IsPCRel },
        { "fixup_sparc_br16_2",    10,      2,  MCFixupKindInfo::FKF_IsPCRel },
        { "fixup_sparc_br16_14",   18,     14,  MCFixupKindInfo::FKF_IsPCRel },
        { "fixup_sparc_hi22",      10,     22,  0 },
        { "fixup_sparc_lo10",      22,     10,  0 },
        { "fixup_sparc_h44",       10,     22,  0 },
        { "fixup_sparc_m44",       22,     10,  0 },
        { "fixup_sparc_l44",       20,     12,  0 },
        { "fixup_sparc_hh",        10,     22,  0 },
        { "fixup_sparc_hm",        22,     10,  0 },
        { "fixup_sparc_pc22",      10,     22,  MCFixupKindInfo::FKF_IsPCRel },
        { "fixup_sparc_pc10",      22,     10,  MCFixupKindInfo::FKF_IsPCRel },
        { "fixup_sparc_got22",     10,     22,  0 },
        { "fixup_sparc_got10",     22,     10,  0 },
        { "fixup_sparc_wplt30",     2,     30,  MCFixupKindInfo::FKF_IsPCRel },
        { "fixup_sparc_tls_gd_hi22",   10, 22,  0 },
        { "fixup_sparc_tls_gd_lo10",   22, 10,  0 },
        { "fixup_sparc_tls_gd_add",     0,  0,  0 },
        { "fixup_sparc_tls_gd_call",    0,  0,  0 },
        { "fixup_sparc_tls_ldm_hi22",  10, 22,  0 },
        { "fixup_sparc_tls_ldm_lo10",  22, 10,  0 },
        { "fixup_sparc_tls_ldm_add",    0,  0,  0 },
        { "fixup_sparc_tls_ldm_call",   0,  0,  0 },
        { "fixup_sparc_tls_ldo_hix22", 10, 22,  0 },
        { "fixup_sparc_tls_ldo_lox10", 22, 10,  0 },
        { "fixup_sparc_tls_ldo_add",    0,  0,  0 },
        { "fixup_sparc_tls_ie_hi22",   10, 22,  0 },
        { "fixup_sparc_tls_ie_lo10",   22, 10,  0 },
        { "fixup_sparc_tls_ie_ld",      0,  0,  0 },
        { "fixup_sparc_tls_ie_ldx",     0,  0,  0 },
        { "fixup_sparc_tls_ie_add",     0,  0,  0 },
        { "fixup_sparc_tls_le_hix22",   0,  0,  0 },
        { "fixup_sparc_tls_le_lox10",   0,  0,  0 }
      };

      const static MCFixupKindInfo InfosLE[V9::NumTargetFixupKinds] = {
        // name                    offset bits  flags
        { "fixup_sparc_call30",     0,     30,  MCFixupKindInfo::FKF_IsPCRel },
        { "fixup_sparc_br22",       0,     22,  MCFixupKindInfo::FKF_IsPCRel },
        { "fixup_sparc_br19",       0,     19,  MCFixupKindInfo::FKF_IsPCRel },
        { "fixup_sparc_br16_2",    20,      2,  MCFixupKindInfo::FKF_IsPCRel },
        { "fixup_sparc_br16_14",    0,     14,  MCFixupKindInfo::FKF_IsPCRel },
        { "fixup_sparc_hi22",       0,     22,  0 },
        { "fixup_sparc_lo10",       0,     10,  0 },
        { "fixup_sparc_h44",        0,     22,  0 },
        { "fixup_sparc_m44",        0,     10,  0 },
        { "fixup_sparc_l44",        0,     12,  0 },
        { "fixup_sparc_hh",         0,     22,  0 },
        { "fixup_sparc_hm",         0,     10,  0 },
        { "fixup_sparc_pc22",       0,     22,  MCFixupKindInfo::FKF_IsPCRel },
        { "fixup_sparc_pc10",       0,     10,  MCFixupKindInfo::FKF_IsPCRel },
        { "fixup_sparc_got22",      0,     22,  0 },
        { "fixup_sparc_got10",      0,     10,  0 },
        { "fixup_sparc_wplt30",      0,     30,  MCFixupKindInfo::FKF_IsPCRel },
        { "fixup_sparc_tls_gd_hi22",    0, 22,  0 },
        { "fixup_sparc_tls_gd_lo10",    0, 10,  0 },
        { "fixup_sparc_tls_gd_add",     0,  0,  0 },
        { "fixup_sparc_tls_gd_call",    0,  0,  0 },
        { "fixup_sparc_tls_ldm_hi22",   0, 22,  0 },
        { "fixup_sparc_tls_ldm_lo10",   0, 10,  0 },
        { "fixup_sparc_tls_ldm_add",    0,  0,  0 },
        { "fixup_sparc_tls_ldm_call",   0,  0,  0 },
        { "fixup_sparc_tls_ldo_hix22",  0, 22,  0 },
        { "fixup_sparc_tls_ldo_lox10",  0, 10,  0 },
        { "fixup_sparc_tls_ldo_add",    0,  0,  0 },
        { "fixup_sparc_tls_ie_hi22",    0, 22,  0 },
        { "fixup_sparc_tls_ie_lo10",    0, 10,  0 },
        { "fixup_sparc_tls_ie_ld",      0,  0,  0 },
        { "fixup_sparc_tls_ie_ldx",     0,  0,  0 },
        { "fixup_sparc_tls_ie_add",     0,  0,  0 },
        { "fixup_sparc_tls_le_hix22",   0,  0,  0 },
        { "fixup_sparc_tls_le_lox10",   0,  0,  0 }
      };

      if (Kind < FirstTargetFixupKind)
        return MCAsmBackend::getFixupKindInfo(Kind);

      assert(unsigned(Kind - FirstTargetFixupKind) < getNumFixupKinds() &&
             "Invalid kind!");
      if (IsLittleEndian)
        return InfosLE[Kind - FirstTargetFixupKind];

      return InfosBE[Kind - FirstTargetFixupKind];
    }

    void processFixupValue(const MCAssembler &Asm, const MCAsmLayout &Layout,
                           const MCFixup &Fixup, const MCFragment *DF,
                           const MCValue &Target, uint64_t &Value,
                           bool &IsResolved) override {
      switch ((V9::Fixups)Fixup.getKind()) {
      default: break;
      case V9::fixup_sparc_wplt30:
        if (Target.getSymA()->getSymbol().isTemporary())
          return;
      case V9::fixup_sparc_tls_gd_hi22:
      case V9::fixup_sparc_tls_gd_lo10:
      case V9::fixup_sparc_tls_gd_add:
      case V9::fixup_sparc_tls_gd_call:
      case V9::fixup_sparc_tls_ldm_hi22:
      case V9::fixup_sparc_tls_ldm_lo10:
      case V9::fixup_sparc_tls_ldm_add:
      case V9::fixup_sparc_tls_ldm_call:
      case V9::fixup_sparc_tls_ldo_hix22:
      case V9::fixup_sparc_tls_ldo_lox10:
      case V9::fixup_sparc_tls_ldo_add:
      case V9::fixup_sparc_tls_ie_hi22:
      case V9::fixup_sparc_tls_ie_lo10:
      case V9::fixup_sparc_tls_ie_ld:
      case V9::fixup_sparc_tls_ie_ldx:
      case V9::fixup_sparc_tls_ie_add:
      case V9::fixup_sparc_tls_le_hix22:
      case V9::fixup_sparc_tls_le_lox10:  IsResolved = false; break;
      }
    }

    bool mayNeedRelaxation(const MCInst &Inst) const override {
      // FIXME.
      return false;
    }

    /// fixupNeedsRelaxation - Target specific predicate for whether a given
    /// fixup requires the associated instruction to be relaxed.
    bool fixupNeedsRelaxation(const MCFixup &Fixup,
                              uint64_t Value,
                              const MCRelaxableFragment *DF,
                              const MCAsmLayout &Layout) const override {
      // FIXME.
      llvm_unreachable("fixupNeedsRelaxation() unimplemented");
      return false;
    }
    void relaxInstruction(const MCInst &Inst, MCInst &Res) const override {
      // FIXME.
      llvm_unreachable("relaxInstruction() unimplemented");
    }

    bool writeNopData(uint64_t Count, MCObjectWriter *OW) const override {
      // Cannot emit NOP with size not multiple of 32 bits.
      if (Count % 4 != 0)
        return false;

      uint64_t NumNops = Count / 4;
      for (uint64_t i = 0; i != NumNops; ++i)
        OW->write32(0x01000000);

      return true;
    }
  };

  class ELFV9AsmBackend : public V9AsmBackend {
    Triple::OSType OSType;
  public:
    ELFV9AsmBackend(const Target &T, Triple::OSType OSType) :
      V9AsmBackend(T), OSType(OSType) { }

    void applyFixup(const MCFixup &Fixup, char *Data, unsigned DataSize,
                    uint64_t Value, bool IsPCRel) const override {

      Value = adjustFixupValue(Fixup.getKind(), Value);
      if (!Value) return;           // Doesn't change encoding.

      unsigned Offset = Fixup.getOffset();

      // For each byte of the fragment that the fixup touches, mask in the bits
      // from the fixup value. The Value has been "split up" into the
      // appropriate bitfields above.
      for (unsigned i = 0; i != 4; ++i) {
        unsigned Idx = IsLittleEndian ? i : 3 - i;
        Data[Offset + Idx] |= uint8_t((Value >> (i * 8)) & 0xff);
      }
    }

    MCObjectWriter *createObjectWriter(raw_pwrite_stream &OS) const override {
      uint8_t OSABI = MCELFObjectTargetWriter::getOSABI(OSType);
      return createV9ELFObjectWriter(OS, Is64Bit, IsLittleEndian, OSABI);
    }
  };

} // end anonymous namespace

MCAsmBackend *llvm::createV9AsmBackend(const Target &T,
                                          const MCRegisterInfo &MRI,
                                          const Triple &TT, StringRef CPU) {
  return new ELFV9AsmBackend(T, TT.getOS());
}
