//====- V9MCExpr.h - V9 specific MC expression classes --*- C++ -*-=====//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file describes V9-specific MCExprs, used for modifiers like
// "%hi" or "%lo" etc.,
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_SPARC_MCTARGETDESC_SPARCMCEXPR_H
#define LLVM_LIB_TARGET_SPARC_MCTARGETDESC_SPARCMCEXPR_H

#include "V9FixupKinds.h"
#include "llvm/MC/MCExpr.h"

namespace llvm {

class StringRef;
class V9MCExpr : public MCTargetExpr {
public:
  enum VariantKind {
    VK_V9_None,
    VK_V9_LO,
    VK_V9_HI,
    VK_V9_H44,
    VK_V9_M44,
    VK_V9_L44,
    VK_V9_HH,
    VK_V9_HM,
    VK_V9_PC22,
    VK_V9_PC10,
    VK_V9_GOT22,
    VK_V9_GOT10,
    VK_V9_WPLT30,
    VK_V9_R_DISP32,
    VK_V9_TLS_GD_HI22,
    VK_V9_TLS_GD_LO10,
    VK_V9_TLS_GD_ADD,
    VK_V9_TLS_GD_CALL,
    VK_V9_TLS_LDM_HI22,
    VK_V9_TLS_LDM_LO10,
    VK_V9_TLS_LDM_ADD,
    VK_V9_TLS_LDM_CALL,
    VK_V9_TLS_LDO_HIX22,
    VK_V9_TLS_LDO_LOX10,
    VK_V9_TLS_LDO_ADD,
    VK_V9_TLS_IE_HI22,
    VK_V9_TLS_IE_LO10,
    VK_V9_TLS_IE_LD,
    VK_V9_TLS_IE_LDX,
    VK_V9_TLS_IE_ADD,
    VK_V9_TLS_LE_HIX22,
    VK_V9_TLS_LE_LOX10
  };

private:
  const VariantKind Kind;
  const MCExpr *Expr;

  explicit V9MCExpr(VariantKind Kind, const MCExpr *Expr)
      : Kind(Kind), Expr(Expr) {}

public:
  /// @name Construction
  /// @{

  static const V9MCExpr *create(VariantKind Kind, const MCExpr *Expr,
                                 MCContext &Ctx);
  /// @}
  /// @name Accessors
  /// @{

  /// getOpcode - Get the kind of this expression.
  VariantKind getKind() const { return Kind; }

  /// getSubExpr - Get the child of this expression.
  const MCExpr *getSubExpr() const { return Expr; }

  /// getFixupKind - Get the fixup kind of this expression.
  V9::Fixups getFixupKind() const { return getFixupKind(Kind); }

  /// @}
  void printImpl(raw_ostream &OS, const MCAsmInfo *MAI) const override;
  bool evaluateAsRelocatableImpl(MCValue &Res,
                                 const MCAsmLayout *Layout,
                                 const MCFixup *Fixup) const override;
  void visitUsedExpr(MCStreamer &Streamer) const override;
  MCFragment *findAssociatedFragment() const override {
    return getSubExpr()->findAssociatedFragment();
  }

  void fixELFSymbolsInTLSFixups(MCAssembler &Asm) const override;

  static bool classof(const MCExpr *E) {
    return E->getKind() == MCExpr::Target;
  }

  static bool classof(const V9MCExpr *) { return true; }

  static VariantKind parseVariantKind(StringRef name);
  static bool printVariantKind(raw_ostream &OS, VariantKind Kind);
  static V9::Fixups getFixupKind(VariantKind Kind);
};

} // end namespace llvm.

#endif
