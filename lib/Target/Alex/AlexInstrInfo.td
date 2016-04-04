class AlexInst<string asmstr, dag outs, dag ins, list<dag> pattern, InstrItinClass itin = NoItinerary>: Instruction
{
  field bits<32> Inst;
  let Namespace = "Alex";
  let Size = 4;
  bits<8> Opcode = 0;
  let Inst{31-24} = Opcode;

  let OutOperandList = outs;
  let InOperandList  = ins;

  let AsmString   = asmstr;
  let Pattern     = pattern;
  let Itinerary   = itin;
}

class InstrRRI<string asmstr, bits<8> op, dag outs, dag ins,
         list<dag> pattern, InstrItinClass itin>:
      AlexInst<asmstr, outs, ins, pattern, itin>
{
  bits<3>  ra;
  bits<3>  rb;
  bits<16> imm16;

  let Opcode = op;

  let Inst{23-21} = ra;
  let Inst{20-18} = rb;
  let Inst{17-16} = 0;
  let Inst{15-0}  = imm16;
}

class InstrRR<string asmstr, bits<8> op, dag outs, dag ins,
         list<dag> pattern, InstrItinClass itin>:
      AlexInst<asmstr, outs, ins, pattern, itin>
{
  bits<3>  ra;
  bits<3>  rb;

  let Opcode = op;

  let Inst{23-21} = ra;
  let Inst{20-18} = rb;
  let Inst{17-0} = 0;
}

class InstrRI<string asmstr, bits<8> op, dag outs, dag ins,
         list<dag> pattern, InstrItinClass itin>:
      AlexInst<asmstr, outs, ins, pattern, itin>
{
  bits<3>  ra;
  bits<16> imm16;

  let Opcode = op;
  let Inst{23-21} = ra;
  let Inst{20-16} = 0b00000;
  let Inst{15-0}  = imm16;
}

class InstrR<string asmstr, bits<8> op, dag outs, dag ins,
         list<dag> pattern, InstrItinClass itin>:
      AlexInst<asmstr, outs, ins, pattern, itin>
{
  bits<3>  ra;

  let Opcode = op;

  let Inst{23-21} = ra;
  let Inst{20-0} = 0;
}

def SExtimm16 : PatLeaf<(imm), [{ return isInt<16>(N->getSExtValue()); }]>;

def LI : InstrRRI<"li\t$ra, $imm", 1,
    (outs Int32Regs:$ra), (ins Operand<i32>:$imm),
    [(set Int32Regs:$ra, (i32 SExtimm16:$imm))], NoItinerary>;