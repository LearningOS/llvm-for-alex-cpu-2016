//===-- AlexMCInstLower.h - Lower MachineInstr to MCInst -------*- C++ -*--===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_ALEX_ALEXMCINSTLOWER_H
#define LLVM_LIB_TARGET_ALEX_ALEXMCINSTLOWER_H
#include "MCTargetDesc/AlexMCExpr.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/Support/Compiler.h"

namespace llvm {
class MCContext;
class MCInst;
class MCOperand;
class MachineInstr;
class MachineFunction;
class AlexAsmPrinter;

//@1 {
/// This class is used to lower an MachineInstr into an MCInst.
class LLVM_LIBRARY_VISIBILITY AlexMCInstLower {
//@2
  typedef MachineOperand::MachineOperandType MachineOperandType;
  MCContext *Ctx;
  AlexAsmPrinter &AsmPrinter;
public:
  AlexMCInstLower(AlexAsmPrinter &asmprinter);
  void Initialize(MCContext* C);
  void Lower(const MachineInstr *MI, MCInst &OutMI) const;
  MCOperand LowerOperand(const MachineOperand& MO, unsigned offset = 0) const;

  void LowerCPLOAD(SmallVector<MCInst, 4>& MCInsts);

  //void LowerCPRESTORE(int64_t Offset, SmallVector<MCInst, 4>& MCInsts);
private:
  MCOperand LowerSymbolOperand(const MachineOperand &MO,
                               MachineOperandType MOTy, unsigned Offset) const;

  MCOperand createSub(MachineBasicBlock *BB1, MachineBasicBlock *BB2,
                      MCSymbolRefExpr::VariantKind Kind) const;
  void lowerLongBranchLUi(const MachineInstr *MI, MCInst &OutMI) const;
  void lowerLongBranchADDiu(const MachineInstr *MI, MCInst &OutMI,
                            int Opcode,
                            MCSymbolRefExpr::VariantKind Kind) const;
  bool lowerLongBranch(const MachineInstr *MI, MCInst &OutMI) const;

};
}

#endif