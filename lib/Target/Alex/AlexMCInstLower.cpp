//===-- AlexMCInstLower.cpp - Convert Alex MachineInstr to MCInst ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains code to lower Alex MachineInstrs to their corresponding
// MCInst records.
//
//===----------------------------------------------------------------------===//

#include "AlexMCInstLower.h"

#include "AlexAsmPrinter.h"
#include "AlexInstrInfo.h"
#include "MCTargetDesc/AlexBaseInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/IR/Mangler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"

using namespace llvm;

AlexMCInstLower::AlexMCInstLower(AlexAsmPrinter &asmprinter)
        : AsmPrinter(asmprinter) {}

void AlexMCInstLower::Initialize(MCContext* C) {
  Ctx = C;
}

//@LowerSymbolOperand {
MCOperand AlexMCInstLower::LowerSymbolOperand(const MachineOperand &MO,
                                              MachineOperandType MOTy,
                                              unsigned Offset) const {
  MCSymbolRefExpr::VariantKind Kind;
  const MCSymbol *Symbol;

  switch(MO.getTargetFlags()) {
  default:                   llvm_unreachable("Invalid target flag!");
  case AlexII::MO_NO_FLAG:   Kind = MCSymbolRefExpr::VK_None; break;

// Alex_GPREL is for llc -march=cpu0 -relocation-model=static -cpu0-islinux-
//  format=false (global var in .sdata).
  //case AlexII::MO_GPREL:     Kind = MCSymbolRefExpr::VK_Alex_GPREL; break;

  //case AlexII::MO_GOT_CALL:  Kind = MCSymbolRefExpr::VK_Alex_GOT_CALL; break;

  //case AlexII::MO_GOT16:     Kind = MCSymbolRefExpr::VK_Alex_GOT16; break;
  //case AlexII::MO_GOT:       Kind = MCSymbolRefExpr::VK_Alex_GOT; break;
// ABS_HI and ABS_LO is for llc -march=cpu0 -relocation-model=static (global 
//  var in .data).
  case AlexII::MO_ABS_HI:    Kind = MCSymbolRefExpr::VK_Alex_ABS_HI; break;
  case AlexII::MO_ABS_LO:    Kind = MCSymbolRefExpr::VK_Alex_ABS_LO; break;

//  case AlexII::MO_TLSGD:     Kind = MCSymbolRefExpr::VK_Alex_TLSGD; break;
//  case AlexII::MO_TLSLDM:    Kind = MCSymbolRefExpr::VK_Alex_TLSLDM; break;
//  case AlexII::MO_DTP_HI:    Kind = MCSymbolRefExpr::VK_Alex_DTP_HI; break;
//  case AlexII::MO_DTP_LO:    Kind = MCSymbolRefExpr::VK_Alex_DTP_LO; break;
//  case AlexII::MO_GOTTPREL:  Kind = MCSymbolRefExpr::VK_Alex_GOTTPREL; break;
//  case AlexII::MO_TP_HI:     Kind = MCSymbolRefExpr::VK_Alex_TP_HI; break;
//  case AlexII::MO_TP_LO:     Kind = MCSymbolRefExpr::VK_Alex_TP_LO; break;
//
//  case AlexII::MO_GOT_HI16:  Kind = MCSymbolRefExpr::VK_Alex_GOT_HI16; break;
//  case AlexII::MO_GOT_LO16:  Kind = MCSymbolRefExpr::VK_Alex_GOT_LO16; break;
  }

  switch (MOTy) {
  case MachineOperand::MO_GlobalAddress:
    Symbol = AsmPrinter.getSymbol(MO.getGlobal());
    break;

  case MachineOperand::MO_MachineBasicBlock:
    Symbol = MO.getMBB()->getSymbol();
    break;

  case MachineOperand::MO_BlockAddress:
    Symbol = AsmPrinter.GetBlockAddressSymbol(MO.getBlockAddress());
    Offset += MO.getOffset();
    break;

  case MachineOperand::MO_ExternalSymbol:
    Symbol = AsmPrinter.GetExternalSymbolSymbol(MO.getSymbolName());
    Offset += MO.getOffset();
    break;

  case MachineOperand::MO_JumpTableIndex:
    Symbol = AsmPrinter.GetJTISymbol(MO.getIndex());
    break;

  default:
    llvm_unreachable("<unknown operand type>");
  }

  const MCSymbolRefExpr *MCSym = MCSymbolRefExpr::create(Symbol, Kind, *Ctx);

  if (!Offset)
    return MCOperand::createExpr(MCSym);

  // Assume offset is never negative.
  assert(Offset > 0);

  const MCConstantExpr *OffsetExpr =  MCConstantExpr::create(Offset, *Ctx);
  const MCBinaryExpr *AddExpr = MCBinaryExpr::createAdd(MCSym, OffsetExpr, *Ctx);
  return MCOperand::createExpr(AddExpr);
}
//@LowerSymbolOperand }

static void CreateMCInst(MCInst& Inst, unsigned Opc, const MCOperand& Opnd0,
                         const MCOperand& Opnd1,
                         const MCOperand& Opnd2 = MCOperand()) {
  Inst.setOpcode(Opc);
  Inst.addOperand(Opnd0);
  Inst.addOperand(Opnd1);
  if (Opnd2.isValid())
    Inst.addOperand(Opnd2);
}

// Lower ".cpload $reg" to
//  "lui   $gp, %hi(_gp_disp)"
//  "addiu $gp, $gp, %lo(_gp_disp)"
//  "addu  $gp, $gp, $t9"
//void AlexMCInstLower::LowerCPLOAD(SmallVector<MCInst, 4>& MCInsts) {
//  MCOperand GPReg = MCOperand::createReg(Alex::GP);
//  MCOperand T9Reg = MCOperand::createReg(Alex::T9);
//  StringRef SymName("_gp_disp");
//  const MCSymbol *Sym = Ctx->getOrCreateSymbol(SymName);
//  const MCSymbolRefExpr *MCSym;
//
//  MCSym = MCSymbolRefExpr::create(Sym, MCSymbolRefExpr::VK_Alex_ABS_HI, *Ctx);
//  MCOperand SymHi = MCOperand::createExpr(MCSym);
//  MCSym = MCSymbolRefExpr::create(Sym, MCSymbolRefExpr::VK_Alex_ABS_LO, *Ctx);
//  MCOperand SymLo = MCOperand::createExpr(MCSym);
//
//  MCInsts.resize(3);
//
//  CreateMCInst(MCInsts[0], Alex::LUi, GPReg, SymHi);
//  CreateMCInst(MCInsts[1], Alex::ORi, GPReg, GPReg, SymLo);
//  CreateMCInst(MCInsts[2], Alex::ADD, GPReg, GPReg, T9Reg);
//}

//#ifdef ENABLE_GPRESTORE
//// Lower ".cprestore offset" to "st $gp, offset($sp)".
//void AlexMCInstLower::LowerCPRESTORE(int64_t Offset,
//                                     SmallVector<MCInst, 4>& MCInsts) {
//  assert(isInt<32>(Offset) && (Offset >= 0) &&
//         "Imm operand of .cprestore must be a non-negative 32-bit value.");
//
//  MCOperand SPReg = MCOperand::createReg(Alex::SP), BaseReg = SPReg;
//  MCOperand GPReg = MCOperand::createReg(Alex::GP);
//  MCOperand ZEROReg = MCOperand::createReg(Alex::ZERO);
//
//  if (!isInt<16>(Offset)) {
//    unsigned Hi = ((Offset + 0x8000) >> 16) & 0xffff;
//    Offset &= 0xffff;
//    MCOperand ATReg = MCOperand::createReg(Alex::AT);
//    BaseReg = ATReg;
//
//    // lui   at,hi
//    // add   at,at,sp
//    MCInsts.resize(2);
//    CreateMCInst(MCInsts[0], Alex::LUi, ATReg, ZEROReg, MCOperand::createImm(Hi));
//    CreateMCInst(MCInsts[1], Alex::ADD, ATReg, ATReg, SPReg);
//  }
//
//  MCInst St;
//  CreateMCInst(St, Alex::ST, GPReg, BaseReg, MCOperand::createImm(Offset));
//  MCInsts.push_back(St);
//}
//#endif

//@LowerOperand {
MCOperand AlexMCInstLower::LowerOperand(const MachineOperand& MO,
                                        unsigned offset) const {
  MachineOperandType MOTy = MO.getType();

  switch (MOTy) {
    //@2
  default: llvm_unreachable("unknown operand type");
  case MachineOperand::MO_Register:
    // Ignore all implicit register operands.
    if (MO.isImplicit()) break;
    return MCOperand::createReg(MO.getReg());
  case MachineOperand::MO_Immediate:
    return MCOperand::createImm(MO.getImm() + offset);
//#if CH >= CH8_1
  case MachineOperand::MO_MachineBasicBlock:
//#endif
//#if CH >= CH9_1 //3
//  case MachineOperand::MO_ExternalSymbol:
//#endif
//#if CH >= CH8_1
//  case MachineOperand::MO_JumpTableIndex:
  case MachineOperand::MO_BlockAddress:
//#endif

  case MachineOperand::MO_GlobalAddress:
//@1
    return LowerSymbolOperand(MO, MOTy, offset);

  case MachineOperand::MO_RegisterMask:
    break;
  }

  return MCOperand();
}
/*
#if CH >= CH8_2 //1
MCOperand AlexMCInstLower::createSub(MachineBasicBlock *BB1,
                                     MachineBasicBlock *BB2,
                                     MCSymbolRefExpr::VariantKind Kind) const {
  const MCSymbolRefExpr *Sym1 = MCSymbolRefExpr::create(BB1->getSymbol(), *Ctx);
  const MCSymbolRefExpr *Sym2 = MCSymbolRefExpr::create(BB2->getSymbol(), *Ctx);
  const MCBinaryExpr *Sub = MCBinaryExpr::createSub(Sym1, Sym2, *Ctx);

  return MCOperand::createExpr(AlexMCExpr::create(Kind, Sub, *Ctx));
}

void AlexMCInstLower::
lowerLongBranchLUi(const MachineInstr *MI, MCInst &OutMI) const {
  OutMI.setOpcode(Alex::LUi);

  // Lower register operand.
  OutMI.addOperand(LowerOperand(MI->getOperand(0)));

  // Create %hi($tgt-$baltgt).
  OutMI.addOperand(createSub(MI->getOperand(1).getMBB(),
                             MI->getOperand(2).getMBB(),
                             MCSymbolRefExpr::VK_Alex_ABS_HI));
}

void AlexMCInstLower::
lowerLongBranchADDiu(const MachineInstr *MI, MCInst &OutMI, int Opcode,
                     MCSymbolRefExpr::VariantKind Kind) const {
  OutMI.setOpcode(Opcode);

  // Lower two register operands.
  for (unsigned I = 0, E = 2; I != E; ++I) {
    const MachineOperand &MO = MI->getOperand(I);
    OutMI.addOperand(LowerOperand(MO));
  }

  // Create %lo($tgt-$baltgt) or %hi($tgt-$baltgt).
  OutMI.addOperand(createSub(MI->getOperand(2).getMBB(),
                             MI->getOperand(3).getMBB(), Kind));
}

bool AlexMCInstLower::lowerLongBranch(const MachineInstr *MI,
                                      MCInst &OutMI) const {
  switch (MI->getOpcode()) {
  default:
    return false;
  case Alex::LONG_BRANCH_LUi:
    lowerLongBranchLUi(MI, OutMI);
    return true;
  case Alex::LONG_BRANCH_ADDiu:
    lowerLongBranchADDiu(MI, OutMI, Alex::ADDiu,
                         MCSymbolRefExpr::VK_Alex_ABS_LO);
    return true;
  }
}
#endif*/

void AlexMCInstLower::Lower(const MachineInstr *MI, MCInst &OutMI) const {
//#if CH >= CH8_2 //2
//  if (lowerLongBranch(MI, OutMI))
//    return;
//#endif
  OutMI.setOpcode(MI->getOpcode());

  for (unsigned i = 0, e = MI->getNumOperands(); i != e; ++i) {
    const MachineOperand &MO = MI->getOperand(i);
    MCOperand MCOp = LowerOperand(MO);

    if (MCOp.isValid())
      OutMI.addOperand(MCOp);
  }
}
