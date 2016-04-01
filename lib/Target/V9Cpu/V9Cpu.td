include "llvm/Target/Target.td"
include "V9CpuRegisterInfo.td"
include "V9CpuCallingConv.td"
include "V9CpuInstrInfo.td"

def V9CpuInstrInfo : InstrInfo;

def V9CpuAsmParser : AsmParser {
}

def V9CpuAsmWriter : AsmWriter {
  string AsmWriterClassName  = "InstPrinter";
}

def V9Cpu : Target {
  let InstructionSet = V9CpuInstrInfo;
}
