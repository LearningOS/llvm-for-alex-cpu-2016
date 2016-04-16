//===-- AlexMCExpr.cpp - Alex specific MC expression classes --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//


#include "AlexMCExpr.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCObjectStreamer.h"

using namespace llvm;

#define DEBUG_TYPE "cpu0mcexpr"

bool AlexMCExpr::isSupportedBinaryExpr(MCSymbolRefExpr::VariantKind VK,
                                       const MCBinaryExpr *BE) {
  switch (VK) {
  case MCSymbolRefExpr::VK_Alex_ABS_LO:
  case MCSymbolRefExpr::VK_Alex_ABS_HI:
    break;
  default:
    return false;
  }

  // We support expressions of the form "(sym1 binop1 sym2) binop2 const",
  // where "binop2 const" is optional.
  if (isa<MCBinaryExpr>(BE->getLHS())) {
    if (!isa<MCConstantExpr>(BE->getRHS()))
      return false;
    BE = cast<MCBinaryExpr>(BE->getLHS());
  }
  return (isa<MCSymbolRefExpr>(BE->getLHS())
          && isa<MCSymbolRefExpr>(BE->getRHS()));
}

const AlexMCExpr*
AlexMCExpr::create(MCSymbolRefExpr::VariantKind VK, const MCExpr *Expr,
                   MCContext &Ctx) {
  VariantKind Kind;
  switch (VK) {
  case MCSymbolRefExpr::VK_Alex_ABS_LO:
    Kind = VK_Alex_LO;
    break;
  case MCSymbolRefExpr::VK_Alex_ABS_HI:
    Kind = VK_Alex_HI;
    break;
  default:
    llvm_unreachable("Invalid kind!");
  }

  return new (Ctx) AlexMCExpr(Kind, Expr);
}

void AlexMCExpr::printImpl(raw_ostream &OS, const MCAsmInfo *MAI) const {
  switch (Kind) {
  default: llvm_unreachable("Invalid kind!");
  case VK_Alex_LO: OS << "%lo"; break;
  case VK_Alex_HI: OS << "%hi"; break;
  }

  OS << '(';
  Expr->print(OS, MAI);
  OS << ')';
}

bool
AlexMCExpr::evaluateAsRelocatableImpl(MCValue &Res,
                                      const MCAsmLayout *Layout,
                                      const MCFixup *Fixup) const {
  return getSubExpr()->evaluateAsRelocatable(Res, Layout, Fixup);
}

void AlexMCExpr::visitUsedExpr(MCStreamer &Streamer) const {
  Streamer.visitUsedExpr(*getSubExpr());
}