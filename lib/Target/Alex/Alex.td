include "llvm/Target/Target.td"
include "AlexRegisterInfo.td"
include "AlexCallingConv.td"
include "AlexInstrInfo.td"

def AlexInstrInfo : instrInfo;

def : Processor<"a1", NoItineraries, []>;

def AlexAsmWriter : AsmWriter {
  string AsmWriterClassName  = "InstPrinter";
  int Variant = 0;
}

def Alex : Target {
  let InstructionSet = AlexInstrInfo;
  let AssemblyWriters = [AlexAsmWriter];
}
