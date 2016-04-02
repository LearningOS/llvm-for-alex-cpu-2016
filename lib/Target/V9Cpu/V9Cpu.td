

include "llvm/Target/Target.td"
include "V9CpuRegisterInfo.td"
include "V9CpuCallingConv.td"
include "V9CpuInstrInfo.td"

def FeatureV9CpuI     : SubtargetFeature<"v9cpui", "V9CpuArchVersion",
                                "V9CpuI", "V9CpuI ISA Support">;

def V9CpuInstrInfo : InstrInfo;

def V9CpuAsmParser : AsmParser {
}

def V9CpuAsmWriter : AsmWriter {
  string AsmWriterClassName  = "InstPrinter";
}

def V9Cpu : Target {
  let InstructionSet = V9CpuInstrInfo;
}

class Proc<string Name, list<SubtargetFeature> Features>
 : Processor<Name, NoItineraries, Features>;

def : Proc<"v9-generic",         [FeatureV9CpuI]>;
