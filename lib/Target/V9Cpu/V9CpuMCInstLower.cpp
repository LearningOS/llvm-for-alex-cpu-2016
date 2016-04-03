//===-- V9CpuMCInstLower.cpp - Convert V9Cpu MachineInstr to MCInst ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains code to lower V9Cpu MachineInstrs to their corresponding
// MCInst records.
//
//===----------------------------------------------------------------------===//

#include "V9CpuMCInstLower.h"

#include "V9CpuAsmPrinter.h"
#include "V9CpuInstrInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/IR/Mangler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"

using namespace llvm;

V9CpuMCInstLower::V9CpuMCInstLower(V9CpuAsmPrinter &asmprinter)
        : AsmPrinter(asmprinter) {}

void V9CpuMCInstLower::Initialize(MCContext* C) {
    Ctx = C;
}

static void CreateMCInst(MCInst& Inst, unsigned Opc, const MCOperand& Opnd0,
                         const MCOperand& Opnd1,
                         const MCOperand& Opnd2 = MCOperand()) {
    Inst.setOpcode(Opc);
    Inst.addOperand(Opnd0);
    Inst.addOperand(Opnd1);
    if (Opnd2.isValid())
        Inst.addOperand(Opnd2);
}
MCOperand V9CpuMCInstLower::LowerSymbolOperand(const MachineOperand &MO,
                                              MachineOperandType MOTy,
                                              unsigned Offset) const {
    MCSymbolRefExpr::VariantKind Kind = MCSymbolRefExpr::VK_None;
    const MCSymbol *Symbol;
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
//@LowerOperand {
MCOperand V9CpuMCInstLower::LowerOperand(const MachineOperand& MO,
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
        case MachineOperand::MO_RegisterMask:
            break;
        case MachineOperand::MO_MachineBasicBlock:
        case MachineOperand::MO_JumpTableIndex:
        case MachineOperand::MO_BlockAddress:
            return LowerSymbolOperand(MO, MOTy, offset);
    }

    return MCOperand();
}

void V9CpuMCInstLower::Lower(const MachineInstr *MI, MCInst &OutMI) const {
    OutMI.setOpcode(MI->getOpcode());

    for (unsigned i = 0, e = MI->getNumOperands(); i != e; ++i) {
        const MachineOperand &MO = MI->getOperand(i);
        MCOperand MCOp = LowerOperand(MO);

        if (MCOp.isValid())
            OutMI.addOperand(MCOp);
    }
}