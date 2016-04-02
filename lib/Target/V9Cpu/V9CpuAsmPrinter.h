#ifndef LLVM_LIB_TARGET_V9CPU_V9CPUASMPRINTER_H
#define LLVM_LIB_TARGET_V9CPU_V9CPUASMPRINTER_H

#include "V9CpuMachineFunction.h"
#include "V9CpuMCInstLower.h"
#include "V9CpuSubtarget.h"
#include "V9CpuTargetMachine.h"
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

class LLVM_LIBRARY_VISIBILITY V9CpuAsmPrinter : public AsmPrinter {

  void EmitInstrWithMacroNoAT(const MachineInstr *MI);

private:

  // lowerOperand - Convert a MachineOperand into the equivalent MCOperand.
  bool lowerOperand(const MachineOperand &MO, MCOperand &MCOp);

public:

  const V9CpuSubtarget *Subtarget;
  const V9CpuFunctionInfo *V9CpuFI;
  V9CpuMCInstLower MCInstLowering;

  explicit V9CpuAsmPrinter(TargetMachine &TM,
                          std::unique_ptr<MCStreamer> Streamer)
    : AsmPrinter(TM, std::move(Streamer)), 
      MCInstLowering(*this) {
    Subtarget = static_cast<V9CpuTargetMachine &>(TM).getSubtargetImpl();
  }

  virtual const char *getPassName() const override {
    return "V9Cpu Assembly Printer";
  }

  virtual bool runOnMachineFunction(MachineFunction &MF) override;

//- EmitInstruction() must exists or will have run time error.
  void EmitInstruction(const MachineInstr *MI) override;
  void printSavedRegsBitmask(raw_ostream &O);
  void printHex32(unsigned int Value, raw_ostream &O);
  void emitFrameDirective();
  const char *getCurrentABIString() const;
  void EmitFunctionEntryLabel() override;
  void EmitFunctionBodyStart() override;
  void EmitFunctionBodyEnd() override;
  void EmitStartOfAsmFile(Module &M) override;
  void PrintDebugValueComment(const MachineInstr *MI, raw_ostream &OS);
};
}

#endif