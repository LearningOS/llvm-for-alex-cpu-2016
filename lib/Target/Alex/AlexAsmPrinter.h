#ifndef LLVM_LIB_TARGET_ALEX_ALEXASMPRINTER_H
#define LLVM_LIB_TARGET_ALEX_ALEXASMPRINTER_H

#include "AlexTargetMachine.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {
    class MCStreamer;

    class MachineInstr;

    class MachineBasicBlock;

    class Module;

    class raw_ostream;

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
        void Initialize(MCContext* C) {
            Ctx = C;
        }
        void Lower(const MachineInstr *MI, MCInst &OutMI) const;
        MCOperand LowerOperand(const MachineOperand& MO, unsigned offset = 0) const;
        MCOperand LowerSymbolOperand(const MachineOperand &MO,
                                     MachineOperandType MOTy,
                                     unsigned Offset) const;

    };

    class LLVM_LIBRARY_VISIBILITY AlexAsmPrinter : public AsmPrinter {
        AlexMCInstLower MCInstLowering;
        void EmitInstrWithMacroNoAT(const MachineInstr *MI);

    private:

        // lowerOperand - Convert a MachineOperand into the equivalent MCOperand.
        bool lowerOperand(const MachineOperand &MO, MCOperand &MCOp);

    public:

        const AlexSubtarget *subtarget;

        explicit AlexAsmPrinter(TargetMachine &targetMachine,
                                std::unique_ptr <MCStreamer> Streamer)
                : AsmPrinter(targetMachine, std::move(Streamer)),
                  subtarget(static_cast<AlexTargetMachine&>(targetMachine).getSubtargetImpl()),
                  MCInstLowering(*this)
        {
        }

        virtual const char *getPassName() const override {
            return "Alex Assembly Printer";
        }

        virtual bool runOnMachineFunction(MachineFunction &MF) override;

        //- EmitInstruction() must exists or will have run time error.
        void EmitInstruction(const MachineInstr *MI) override;

        void printSavedRegsBitmask(raw_ostream &O);

        void printHex32(unsigned int Value, raw_ostream &O);

        void emitFrameDirective();

        void EmitFunctionEntryLabel() override;

        void EmitFunctionBodyStart() override;

        void EmitFunctionBodyEnd() override;

        void EmitStartOfAsmFile(Module &M) override;

        void PrintDebugValueComment(const MachineInstr *MI, raw_ostream &OS);
    };

}

#endif