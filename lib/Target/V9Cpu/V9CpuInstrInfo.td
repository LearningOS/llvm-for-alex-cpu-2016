class Inst<string asmstr, dag outs, dag ins>
          : Instruction {
  field bits<32> Inst;

  let Namespace = "SP";
  let Size = 4;

  dag OutOperandList = outs;
  dag InOperandList = ins;
  let AsmString   = asmstr;
  //let Pattern = pattern;

  let DecoderNamespace = "V9Cpu";
}

class LoadInst<string asmstr, bits<8> opcode>
        : Inst<!strconcat(asmstr, " $src"), (outs), (ins Operand<iPTR>:$src)> {

  let Inst{7-0}  = opcode;
}

def LL   : LoadInst<"ll",  0x0e>;