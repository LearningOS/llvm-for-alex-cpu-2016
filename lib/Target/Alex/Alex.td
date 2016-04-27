include "llvm/Target/Target.td"
include "AlexRegisterInfo.td"
include "AlexCallingConv.td"
include "AlexInstrInfo.td"
include "AlexInstrInfoV9.td"

def AlexInstrInfo : instrInfo;

def : Processor<"a1", NoItineraries, []>;

def AlexAsmWriter : AsmWriter {
  string AsmWriterClassName  = "InstPrinter";
  int Variant = 0;
}

def AlexAsmParser : AsmParser {
  let ShouldEmitMatchRegisterName = 0;
}

def AlexAsmParserVariant : AsmParserVariant {
  int Variant = 0;

  // Recognize hard coded registers.
  string RegisterPrefix = "$";
}

def Alex : Target {
  let InstructionSet = AlexInstrInfo;
  let AssemblyWriters = [AlexAsmWriter];
  let AssemblyParsers = [AlexAsmParser];
    let AssemblyParserVariants = [AlexAsmParserVariant];
}

