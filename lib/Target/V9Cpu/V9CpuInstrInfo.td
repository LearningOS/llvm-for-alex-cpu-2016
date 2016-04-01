class InstSP<string asmstr, dag outs, dag ins>
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

class FA<string asmstr, bits<8> opcode, bits<24> imm, dag outs, dag ins>
        : InstSP<asmstr> {

  let Inst{31-8} = imm;
  let Inst{7-0}  = opcode;
}

def LD   : FA<"lla",  51, 0, (outs), (ins)>;