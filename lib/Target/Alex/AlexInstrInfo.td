/*class AlexInst<string asmstr, dag outs, dag ins, list<dag> pattern, InstrItinClass itin = NoItinerary>: Instruction
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
         list<dag> pattern, InstrItinClass itin = NoItinerary>:
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
         list<dag> pattern, InstrItinClass itin = NoItinerary>:
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
         list<dag> pattern, InstrItinClass itin = NoItinerary>:
      AlexInst<asmstr, outs, ins, pattern, itin>
{
  bits<3>  ra;
  bits<16> imm16;

  let Opcode = op;
  let Inst{23-21} = ra;
  let Inst{20-16} = 0;
  let Inst{15-0}  = imm16;
}

class InstrR<string asmstr, bits<8> op, dag outs, dag ins,
         list<dag> pattern, InstrItinClass itin = NoItinerary>:
      AlexInst<asmstr, outs, ins, pattern, itin>
{
  bits<3>  ra;

  let Opcode = op;

  let Inst{23-21} = ra;
  let Inst{20-0} = 0;
}
class InstrI<string asmstr, bits<8> op, dag outs, dag ins,
         list<dag> pattern, InstrItinClass itin = NoItinerary>:
      AlexInst<asmstr, outs, ins, pattern, itin>
{
  bits<16> imm16;
  let Opcode = op;

  let Inst{23-16} = 0;
  let Inst{15-0} = imm16;
}
class InstrPlain<string asmstr, bits<8> op, dag outs, dag ins,
         list<dag> pattern, InstrItinClass itin = NoItinerary>:
      AlexInst<asmstr, outs, ins, pattern, itin>
{
  let Opcode = op;

  let Inst{23-0} = 0;
}

def IIAlu              : InstrItinClass;
def II_CLO             : InstrItinClass;
def II_CLZ             : InstrItinClass;
def IILoad             : InstrItinClass;
def IIStore            : InstrItinClass;
def IIBranch           : InstrItinClass;

def IIPseudo           : InstrItinClass;
def SExtimm16 : PatLeaf<(imm), [{ return isInt<16>(N->getSExtValue()); }]>;

def LI : InstrRRI<"li\t$ra, $imm", 1,
    (outs Int32Regs:$ra), (ins Operand<i32>:$imm),
    [(set Int32Regs:$ra, (i32 SExtimm16:$imm))], NoItinerary>;

def ADDiu : InstrRRI<"addiu\t$dst, $src, $imm16", 2,
    (outs Int32Regs:$dst), (ins Int32Regs:$src, Operand<i32>:$imm16),
    [(set Int32Regs:$dst, (add Int32Regs:$src, SExtimm16:$imm16))], NoItinerary>;

def JMP : InstrR<"jmp\t$ra", 3,
    (outs), (ins Int32Regs:$ra),
    [], NoItinerary>;

def AlexRet : SDNode<"AlexISD::Ret", SDTNone,
                     [SDNPHasChain]>;
def RetLR : InstrPlain<"ret", 4, (outs), (ins), [(AlexRet)], NoItinerary>;

def addr :ComplexPattern<iPTR, 2, "SelectAddr", [frameindex], [SDNPWantParent]>;
def LD     : InstrRI<"ld", 8, (outs Int32Regs:$ra), (ins Operand<i32>:$imm16),
                [(set Int32Regs:$ra, (load addr:$imm16))]>;

def ST     : InstrRI<"ld", 9, (outs), (ins Int32Regs:$ra, Operand<i32>:$imm16),
                                    [(store Int32Regs:$ra, addr:$imm16)], IIAlu>;

def : Pat<(i32 SExtimm16:$in),
          (LI imm:$in)>;
*/

//===-- AlexInstrFormats.td - Alex Instruction Formats -----*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
//  Describe CPU0 instructions format
//
//  CPU INSTRUCTION FORMATS
//
//  opcode  - operation code.
//  ra      - dst reg, only used on 3 regs instr.
//  rb      - src reg.
//  rc      - src reg (on a 3 reg instr).
//  cx      - immediate
//
//===----------------------------------------------------------------------===//

// Format specifies the encoding used by the instruction.  This is part of the
// ad-hoc solution used to emit machine instruction encodings by our machine
// code emitter.
class Format<bits<4> val> {
  bits<4> Value = val;
}

def Pseudo    : Format<0>;
def FrmA      : Format<1>;
def FrmL      : Format<2>;
def FrmJ      : Format<3>;
def FrmOther  : Format<4>; // Instruction w/ a custom format

// Generic Alex Format
class AlexInst<dag outs, dag ins, string asmstr, list<dag> pattern,
               InstrItinClass itin, Format f>: Instruction
{
  field bits<32> Inst;
  Format Form = f;

  let Namespace = "Alex";

  let Size = 4;

  bits<8> Opcode = 0;

  // Top 8 bits are the 'opcode' field
  let Inst{31-24} = Opcode;

  let OutOperandList = outs;
  let InOperandList  = ins;

  let AsmString   = asmstr;
  let Pattern     = pattern;
  let Itinerary   = itin;

  //
  // Attributes specific to Alex instructions...
  //
  bits<4> FormBits = Form.Value;

  // TSFlags layout should be kept in sync with AlexInstrInfo.h.
  let TSFlags{3-0}   = FormBits;

  let DecoderNamespace = "Alex";

  field bits<32> SoftFail = 0;
}

// Alex Pseudo Instructions Format
class AlexPseudo<bits<8> opc, dag outs, dag ins, string asmstr, list<dag> pattern>:
      AlexInst<outs, ins, asmstr, pattern, NoItinerary, Pseudo> {
  let Opcode = opc;
  let isCodeGenOnly = 1;
  let isPseudo = 1;
}

class InstrR<bits<8> op, dag outs, dag ins, string asmstr, list<dag> pattern,
         InstrItinClass itin>: AlexInst<outs, ins, asmstr, pattern, itin, FrmL>
{
  bits<3>  ra;

  let Opcode = op;

  let Inst{23-21} = ra;
  let Inst{20-0} = 0;
}
class InstrRI<bits<8> op, dag outs, dag ins, string asmstr, list<dag> pattern,
         InstrItinClass itin>: AlexInst<outs, ins, asmstr, pattern, itin, FrmL>
{
  bits<3>  ra;
  bits<16> imm16;

  let Opcode = op;

  let Inst{23-21} = ra;
  let Inst{20-16} = 0;
  let Inst{15-0}  = imm16;
}
class InstrRRI<bits<8> op, dag outs, dag ins, string asmstr, list<dag> pattern,
         InstrItinClass itin>: AlexInst<outs, ins, asmstr, pattern, itin, FrmL>
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
class InstrRRR<bits<8> op, dag outs, dag ins, string asmstr, list<dag> pattern,
         InstrItinClass itin>: AlexInst<outs, ins, asmstr, pattern, itin, FrmL>
{
  bits<3>  ra;
  bits<3>  rb;
  bits<3>  rc;

  let Opcode = op;

  let Inst{23-21} = ra;
  let Inst{20-18} = rb;
  let Inst{17-15} = rc;
  let Inst{14-0}  = 0;
}

class InstrI<bits<8> op, dag outs, dag ins, string asmstr, list<dag> pattern,
         InstrItinClass itin>: AlexInst<outs, ins, asmstr, pattern, itin, FrmJ>
{
  bits<16> imm;

  let Opcode = op;

  let Inst{23-16} = 0;
  let Inst{15-0} = imm;
}

//===- AlexInstrInfo.td - Target Description for Alex Target -*- tablegen -*-=//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the Alex implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Alex profiles and nodes
//===----------------------------------------------------------------------===//

def SDT_AlexRet          : SDTypeProfile<0, 1, [SDTCisInt<0>]>;

// Return
def AlexRet : SDNode<"AlexISD::Ret", SDTNone,
                     [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;

//===----------------------------------------------------------------------===//
// Alex Instruction Predicate Definitions.
//===----------------------------------------------------------------------===//

def EnableOverflow  : Predicate<"Subtarget->enableOverflow()">;
def DisableOverflow : Predicate<"Subtarget->disableOverflow()">;

def HasCmp      :     Predicate<"Subtarget->hasCmp()">;
def HasSlt      :     Predicate<"Subtarget->hasSlt()">;

class AlexInstAlias<string Asm, dag Result, bit Emit = 0b1> :
  InstAlias<Asm, Result, Emit>;

//===----------------------------------------------------------------------===//
// Instruction format superclass
//===----------------------------------------------------------------------===//


//===----------------------------------------------------------------------===//
// Alex Operand, Complex Patterns and Transformations Definitions.
//===----------------------------------------------------------------------===//
// Instruction operand types

// Signed Operand
def simm16      : Operand<i32> {
  let DecoderMethod= "DecodeSimm16";
}

def shamt       : Operand<i32>;

// Unsigned Operand
def uimm16      : Operand<i32> {
  let PrintMethod = "printUnsignedImm";
}

// Address operand
def mem : Operand<iPTR> {
  let PrintMethod = "printMemOperand";
  let MIOperandInfo = (ops Int32Regs, simm16);
  let EncoderMethod = "getMemEncoding";
}

// Transformation Function - get the lower 16 bits.
def LO16 : SDNodeXForm<imm, [{
  return getImm(N, N->getZExtValue() & 0xffff);
}]>;

// Transformation Function - get the higher 16 bits.
def HI16 : SDNodeXForm<imm, [{
  return getImm(N, (N->getZExtValue() >> 16) & 0xffff);
}]>;

// Node immediate fits as 16-bit sign extended on target immediate.
// e.g. addi, andi
def immSExt16  : PatLeaf<(imm), [{ return isInt<16>(N->getSExtValue()); }]>;

// Node immediate fits as 16-bit zero extended on target immediate.
// The LO16 param means that only the lower 16 bits of the node
// immediate are caught.
// e.g. addiu, sltiu
def immZExt16  : PatLeaf<(imm), [{
  if (N->getValueType(0) == MVT::i32)
    return (uint32_t)N->getZExtValue() == (unsigned short)N->getZExtValue();
  else
    return (uint64_t)N->getZExtValue() == (unsigned short)N->getZExtValue();
}], LO16>;

// Immediate can be loaded with LUi (32-bit int with lower 16-bit cleared).
def immLow16Zero : PatLeaf<(imm), [{
  int64_t Val = N->getSExtValue();
  return isInt<32>(Val) && !(Val & 0xffff);
}]>;

// shamt field must fit in 5 bits.
def immZExt5 : ImmLeaf<i32, [{return Imm == (Imm & 0x1f);}]>;

// Alex Address Mode! SDNode frameindex could possibily be a match
// since load and store instructions from stack used it.
def addr :
  ComplexPattern<iPTR, 2, "SelectAddr", [frameindex], [SDNPWantParent]>;

//===----------------------------------------------------------------------===//
// Pattern fragment for load/store
//===----------------------------------------------------------------------===//

class AlignedLoad<PatFrag Node> :
  PatFrag<(ops node:$ptr), (Node node:$ptr), [{
  LoadSDNode *LD = cast<LoadSDNode>(N);
  return LD->getMemoryVT().getSizeInBits()/8 <= LD->getAlignment();
}]>;

class AlignedStore<PatFrag Node> :
  PatFrag<(ops node:$val, node:$ptr), (Node node:$val, node:$ptr), [{
  StoreSDNode *SD = cast<StoreSDNode>(N);
  return SD->getMemoryVT().getSizeInBits()/8 <= SD->getAlignment();
}]>;

// Load/Store PatFrags.
def load_a          : AlignedLoad<load>;
def store_a         : AlignedStore<store>;

//===----------------------------------------------------------------------===//
// Instructions specific format
//===----------------------------------------------------------------------===//


// Arithmetic and logical instructions with 2 register operands.
class ArithLogicRRI<bits<8> op, string instr_asm, SDNode OpNode,
                  Operand Od, PatLeaf imm_type, RegisterClass RC> :
  InstrRRI<op, (outs Int32Regs:$ra), (ins RC:$rb, Od:$imm16),
     !strconcat(instr_asm, "\t$ra, $rb, $imm16"),
     [(set Int32Regs:$ra, (OpNode RC:$rb, imm_type:$imm16))], NoItinerary> {
  let isReMaterializable = 1;
}

class ArithLogicRRR<bits<8> op, string instr_asm, SDNode OpNode, RegisterClass RC> :
  InstrRRR<op, (outs Int32Regs:$ra), (ins RC:$rb, RC:$rc),
     !strconcat(instr_asm, "\t$ra, $rb, $rc"),
     [(set Int32Regs:$ra, (OpNode RC:$rb, RC:$rc))], NoItinerary> {
  let isReMaterializable = 1;
}

class FMem<bits<8> op, dag outs, dag ins, string asmstr, list<dag> pattern,
          InstrItinClass itin>: InstrRRI<op, outs, ins, asmstr, pattern, itin> {
  bits<16> addr;
  let Inst{15-0}  = addr{15-0};
  let DecoderMethod = "DecodeMem";
}

// Memory Load/Store
let canFoldAsLoad = 1 in
class LoadM<bits<8> op, string instr_asm, PatFrag OpNode, RegisterClass RC,
            Operand MemOpnd, bit Pseudo>:
  FMem<op, (outs RC:$ra), (ins MemOpnd:$addr),
     !strconcat(instr_asm, "\t$ra, $addr"),
     [(set RC:$ra, (OpNode addr:$addr))], NoItinerary> {
  let isPseudo = Pseudo;
}

class StoreM<bits<8> op, string instr_asm, PatFrag OpNode, RegisterClass RC,
             Operand MemOpnd, bit Pseudo>:
  FMem<op, (outs), (ins RC:$ra, MemOpnd:$addr),
     !strconcat(instr_asm, "\t$ra, $addr"),
     [(OpNode RC:$ra, addr:$addr)], NoItinerary> {
  let isPseudo = Pseudo;
}

//@ 32-bit load.
multiclass LoadM32<bits<8> op, string instr_asm, PatFrag OpNode,
                   bit Pseudo = 0> {
  def #NAME# : LoadM<op, instr_asm, OpNode, Int32Regs, mem, Pseudo>;
}

// 32-bit store.
multiclass StoreM32<bits<8> op, string instr_asm, PatFrag OpNode,
                    bit Pseudo = 0> {
  def #NAME# : StoreM<op, instr_asm, OpNode, Int32Regs, mem, Pseudo>;
}

//@JumpFR {
let isBranch=1, isTerminator=1, isBarrier=1, imm16=0, hasDelaySlot = 1,
    isIndirectBranch = 1 in
class JumpFR<bits<8> op, string instr_asm, RegisterClass RC>:
  InstrRRI<op, (outs), (ins RC:$ra),
     !strconcat(instr_asm, "\t$ra"), [(brind RC:$ra)], NoItinerary> {
  let rb = 0;
  let imm16 = 0;
}
//@JumpFR }

// Return instruction
class RetBase<RegisterClass RC>: JumpFR<0x3c, "ret", RC> {
  let isReturn = 1;
  let isCodeGenOnly = 1;
  let hasCtrlDep = 1;
  let hasExtraSrcRegAllocReq = 1;
}

//===----------------------------------------------------------------------===//
// Instruction definition
//===----------------------------------------------------------------------===//

def ADDiu   : ArithLogicRRI<4, "addiu", add, simm16, immSExt16, Int32Regs>;
def ADDu    : ArithLogicRRR<3, "addu", add, Int32Regs>;

defm LW     : LoadM32<0x01,  "lw",  load_a>;
defm SW     : StoreM32<0x02, "sw",  store_a>;

def LI      : InstrRI<5, (outs Int32Regs:$ra), (ins simm16:$imm16), "li\t$ra, $imm16",
    [(set Int32Regs:$ra, (i32 immSExt16:$imm16))], NoItinerary>;

//def JR      : JumpFR<0x3c, "jr", Int32Regs>;
def JRRA    : InstrR<7, (outs), (ins Int32Regs:$ra), "jrra\t$ra", [], NoItinerary>;

let isReturn=1, isTerminator=1, hasDelaySlot=1, isBarrier=1, hasCtrlDep=1 in
    def RetLR : AlexPseudo<123, (outs), (ins), "", [(AlexRet)]>;

def RET     : RetBase<Int32Regs>;

// Small immediates
def : Pat<(i32 immSExt16:$in),
          (LI imm:$in)>;