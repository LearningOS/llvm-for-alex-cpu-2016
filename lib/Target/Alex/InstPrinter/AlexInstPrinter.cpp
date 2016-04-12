#include "AlexInstPrinter.h"

#include "AlexInstrInfo.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

#define DEBUG_TYPE "asm-printer"

#define PRINT_ALIAS_INSTR
#include "AlexGenAsmWriter.inc"
AlexInstPrinter::AlexInstPrinter(const MCAsmInfo &MAI, const MCInstrInfo &MII,
                const MCRegisterInfo &MRI)
        : MCInstPrinter(MAI, MII, MRI) {}
void AlexInstPrinter::printRegName(raw_ostream &OS, unsigned RegNo) const {
    OS << '$' << StringRef(getRegisterName(RegNo)).lower();
}



//@1 {
void AlexInstPrinter::printInst(const MCInst *MI, raw_ostream &O,
                                 StringRef Annot, const MCSubtargetInfo &STI) {
    // Try to print any aliases first.
    //if (!printAliasInstr(MI, O))
//@1 }
        //- printInstruction(MI, O) defined in AlexGenAsmWriter.inc which came from 
        //   Alex.td indicate.
        printInstruction(MI, O);
    printAnnotation(O, Annot);
}

//@printExpr {
static void printExpr(const MCExpr *Expr, const MCAsmInfo *MAI,
                      raw_ostream &OS) {
//@printExpr body {
    int Offset = 0;
    const MCSymbolRefExpr *SRE;

    if (const MCBinaryExpr *BE = dyn_cast<MCBinaryExpr>(Expr)) {
        SRE = dyn_cast<MCSymbolRefExpr>(BE->getLHS());
        const MCConstantExpr *CE = dyn_cast<MCConstantExpr>(BE->getRHS());
        assert(SRE && CE && "Binary expression must be sym+const.");
        Offset = CE->getValue();
    } else
        SRE = cast<MCSymbolRefExpr>(Expr);

    MCSymbolRefExpr::VariantKind Kind = SRE->getKind();

    switch (Kind) {
        default:                                 llvm_unreachable("Invalid kind!");
        case MCSymbolRefExpr::VK_None:           break;
    }

    SRE->getSymbol().print(OS, MAI);

    if (Offset) {
        if (Offset > 0)
            OS << '+';
        OS << Offset;
    }

//    if ((Kind == MCSymbolRefExpr::VK_Alex_GPOFF_HI) ||
//        (Kind == MCSymbolRefExpr::VK_Alex_GPOFF_LO))
//        OS << ")))";
//    else if (Kind != MCSymbolRefExpr::VK_None)
//        OS << ')';
}

void AlexInstPrinter::printOperand(const MCInst *MI, unsigned OpNo,
                                    raw_ostream &O) {
    const MCOperand &Op = MI->getOperand(OpNo);
    if (Op.isReg()) {
        printRegName(O, Op.getReg());
        return;
    }

    if (Op.isImm()) {
        O << Op.getImm();
        return;
    }

    assert(Op.isExpr() && "unknown operand kind in printOperand");
    printExpr(Op.getExpr(), &MAI, O);
}

//void AlexInstPrinter::printUnsignedImm(const MCInst *MI, int opNum,
//                                        raw_ostream &O) {
//    const MCOperand &MO = MI->getOperand(opNum);
//    if (MO.isImm())
//        O << (unsigned short int)MO.getImm();
//    else
//        printOperand(MI, opNum, O);
//}
//
void AlexInstPrinter::printMemOperand(const MCInst *MI, int opNum, raw_ostream &O) {
    // Load/Store memory operands -- imm($reg)
    // If PIC target the target is loaded as the
    // pattern ld $t9,%call16($gp)
    printOperand(MI, opNum+1, O);
    O << "(";
    printOperand(MI, opNum, O);
    O << ")";
}