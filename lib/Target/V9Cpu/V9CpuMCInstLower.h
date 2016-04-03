#ifndef LLVM_LIB_TARGET_V9CPU_V9CPUMCINSTLOWER_H
#define LLVM_LIB_TARGET_V9CPU_V9CPUMCINSTLOWER_H

#include "llvm/ADT/SmallVector.h"
#include "llvm/CodeGen/MachineOperand.h"
#include "llvm/Support/Compiler.h"

namespace llvm {
    class MCContext;
    class MCInst;
    class MCOperand;
    class MachineInstr;
    class MachineFunction;
    class V9CpuAsmPrinter;

//@1 {
/// This class is used to lower an MachineInstr into an MCInst.
    class LLVM_LIBRARY_VISIBILITY V9CpuMCInstLower {
        typedef MachineOperand::MachineOperandType MachineOperandType;
        MCOperand LowerSymbolOperand(const MachineOperand &MO,
                                                       MachineOperandType MOTy,
                                                       unsigned Offset) const;
        MCContext *Ctx;
        V9CpuAsmPrinter &AsmPrinter;
    public:
        V9CpuMCInstLower(V9CpuAsmPrinter &asmprinter);
        void Initialize(MCContext* C);
        void Lower(const MachineInstr *MI, MCInst &OutMI) const;
        MCOperand LowerOperand(const MachineOperand& MO, unsigned offset = 0) const;
    };
}

#endif