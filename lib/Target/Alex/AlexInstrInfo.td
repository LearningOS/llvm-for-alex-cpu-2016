//===-- AlexInstrFormats.td - Alex Instruction Formats -----*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
//  Describe Alex instructions format
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

  bits<7> Opcode = 0;

  // Top 8 bits are the 'opcode' field
  let Inst{31-25} = Opcode;

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
class AlexPseudo<bits<7> opc, dag outs, dag ins, string asmstr, list<dag> pattern>:
      AlexInst<outs, ins, asmstr, pattern, NoItinerary, Pseudo> {
  let Opcode = opc;
  let isCodeGenOnly = 1;
  let isPseudo = 1;
}

class InstrPlain<bits<7> op, string asmstr, list<dag> pattern,
         InstrItinClass itin>: AlexInst<(outs), (ins), asmstr, pattern, itin, FrmL>
{
  let Opcode = op;
  let Inst{24-0} = 0;
}
class InstrR<bits<7> op, dag outs, dag ins, string asmstr, list<dag> pattern,
         InstrItinClass itin>: AlexInst<outs, ins, asmstr, pattern, itin, FrmL>
{
  bits<3>  ra;

  let Opcode = op;

  let Inst{24-22} = ra;
  let Inst{21-0} = 0;
}
class InstrRI<bits<7> op, dag outs, dag ins, string asmstr, list<dag> pattern,
         InstrItinClass itin>: AlexInst<outs, ins, asmstr, pattern, itin, FrmL>
{
  bits<3>  ra;
  bits<16> imm16;

  let Opcode = op;

  let Inst{24-22} = ra;
  let Inst{21-16} = 0;
  let Inst{15-0}  = imm16;
}
class InstrRRI<bits<7> op, dag outs, dag ins, string asmstr, list<dag> pattern,
         InstrItinClass itin>: AlexInst<outs, ins, asmstr, pattern, itin, FrmL>
{
  bits<3>  ra;
  bits<3>  rb;
  bits<16> imm16;

  let Opcode = op;

  let Inst{24-22} = ra;
  let Inst{21-19} = rb;
  let Inst{18-16} = 0;
  let Inst{15-0}  = imm16;
}
class InstrRRR<bits<7> op, dag outs, dag ins, string asmstr, list<dag> pattern,
         InstrItinClass itin>: AlexInst<outs, ins, asmstr, pattern, itin, FrmL>
{
  bits<3>  ra;
  bits<3>  rb;
  bits<3>  rc;

  let Opcode = op;

  let Inst{24-22} = ra;
  let Inst{21-19} = rb;
  let Inst{18-16} = rc;
  let Inst{15-0}  = 0;
}

class InstrI<bits<7> op, dag outs, dag ins, string asmstr, list<dag> pattern,
         InstrItinClass itin>: AlexInst<outs, ins, asmstr, pattern, itin, FrmJ>
{
  bits<16> imm;

  let Opcode = op;

  let Inst{24-16} = 0;
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
def SDT_AlexPush         : SDTypeProfile<0, 1, [SDTCisInt<0>]>;
def SDT_AlexPop          : SDTypeProfile<1, 0, [SDTCisInt<0>]>;
def SDT_AlexLI32         : SDTypeProfile<1, 1, [SDTCisInt<0>]>;
def AlexPush : SDNode<"AlexISD::Push", SDT_AlexPush,
                    [SDNPHasChain]>;
def AlexPop  : SDNode<"AlexISD::Pop", SDT_AlexPop,
                    [SDNPHasChain]>;
// Return
def AlexRet : SDNode<"AlexISD::Ret", SDTNone,
                     [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;
def AlexHi    : SDNode<"AlexISD::Hi", SDTIntUnaryOp>;
def AlexLo    : SDNode<"AlexISD::Lo", SDTIntUnaryOp>;
def AlexLI32  : SDNode<"AlexISD::LI32", SDT_AlexLI32>;
//===----------------------------------------------------------------------===//
// Alex Instruction Predicate Definitions.
//===----------------------------------------------------------------------===//

def EnableOverflow  : Predicate<"Subtarget->enableOverflow()">;
def DisableOverflow : Predicate<"Subtarget->disableOverflow()">;

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
def simm32      : Operand<i32> {
  let DecoderMethod= "DecodeSimm32";
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

def memaddr : Operand<iPTR> {
  let PrintMethod = "printMemAddrOperand";
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
def immSExt32  : PatLeaf<(imm), [{ return isInt<32>(N->getSExtValue()); }]>;

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

class AlignedLea<PatFrag Node> :
  PatFrag<(ops node:$ptr), (Node node:$ptr), [{ return true; }]>;

// Load/Store PatFrags.
def load_a          : AlignedLoad<load>;
def store_a         : AlignedStore<store>;

//===----------------------------------------------------------------------===//
// Instructions specific format
//===----------------------------------------------------------------------===//


// Arithmetic and logical instructions with 2 register operands.
class ArithLogicRRI<bits<7> op, string instr_asm, SDNode OpNode,
                  Operand Od, PatLeaf imm_type, RegisterClass RC> :
  InstrRRI<op, (outs Int32Regs:$ra), (ins RC:$rb, Od:$imm16),
     !strconcat(instr_asm, "\t$ra, $rb, $imm16"),
     [(set Int32Regs:$ra, (OpNode RC:$rb, imm_type:$imm16))], NoItinerary> {
  let isReMaterializable = 1;
}

class ArithLogicRRR<bits<7> op, string instr_asm, SDNode OpNode, RegisterClass RC> :
  InstrRRR<op, (outs Int32Regs:$ra), (ins RC:$rb, RC:$rc),
     !strconcat(instr_asm, "\t$ra, $rb, $rc"),
     [(set Int32Regs:$ra, (OpNode RC:$rb, RC:$rc))], NoItinerary> {
  let isReMaterializable = 1;
}
class CompareRRR<bits<7> op, string instr_asm, PatFrag CmpNode, RegisterClass RC> :
  InstrRRR<op, (outs Int32Regs:$ra), (ins RC:$rb, RC:$rc),
     !strconcat(instr_asm, "\t$ra, $rb, $rc"),
     [(set RC:$ra, (CmpNode RC:$rb, RC:$rc))], NoItinerary> {
  let isReMaterializable = 1;
}


class FMem<bits<7> op, dag outs, dag ins, string asmstr, list<dag> pattern,
          InstrItinClass itin>: InstrRRI<op, outs, ins, asmstr, pattern, itin> {
  bits<16> addr;
  let Inst{15-0}  = addr{15-0};
  let DecoderMethod = "DecodeMem";
}

// Memory Load/Store
let canFoldAsLoad = 1 in
class LoadM<bits<7> op, string instr_asm, PatFrag OpNode, RegisterClass RC,
            Operand MemOpnd, bit Pseudo>:
  FMem<op, (outs RC:$ra), (ins MemOpnd:$addr),
     !strconcat(instr_asm, "\t$ra, $addr"),
     [(set RC:$ra, (OpNode addr:$addr))], NoItinerary> {
  let isPseudo = Pseudo;
}

class LeaM<bits<7> op, string instr_asm, RegisterClass RC,
            Operand MemOpnd, bit Pseudo>:
  FMem<op, (outs RC:$ra), (ins MemOpnd:$addr),
     !strconcat(instr_asm, "\t$ra, $addr"),
     [(set RC:$ra, addr:$addr)], NoItinerary> {
  let isPseudo = Pseudo;
}

class StoreM<bits<7> op, string instr_asm, PatFrag OpNode, RegisterClass RC,
             Operand MemOpnd, bit Pseudo>:
  FMem<op, (outs), (ins RC:$ra, MemOpnd:$addr),
     !strconcat(instr_asm, "\t$addr, $ra"),
     [(OpNode RC:$ra, addr:$addr)], NoItinerary> {
  let isPseudo = Pseudo;
}

//@ 32-bit load.
multiclass LoadM32<bits<7> op, string instr_asm, PatFrag OpNode,
                   bit Pseudo = 0> {
  def #NAME# : LoadM<op, instr_asm, OpNode, Int32Regs, mem, Pseudo>;
}
multiclass LeaM32<bits<7> op, string instr_asm,
                   bit Pseudo = 0> {
  def #NAME# : LeaM<op, instr_asm, Int32Regs, memaddr, Pseudo>;
}

// 32-bit store.
multiclass StoreM32<bits<7> op, string instr_asm, PatFrag OpNode,
                    bit Pseudo = 0> {
  def #NAME# : StoreM<op, instr_asm, OpNode, Int32Regs, mem, Pseudo>;
}

//@JumpFR {
let isBranch=1, isTerminator=1, isBarrier=1, imm16=0, hasDelaySlot = 1,
    isIndirectBranch = 1 in
class JumpFR<bits<7> op, string instr_asm, RegisterClass RC>:
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

/* 算数逻辑指令 */

def ADDi    : ArithLogicRRI<0b0000010, "addi",  add,  simm16, immSExt16, Int32Regs>;
def ADDiu   : ArithLogicRRI<0b0000100, "addiu", add,  uimm16, immZExt16, Int32Regs>;
def SUBi    : ArithLogicRRI<0b0000101, "subi",  sub,  simm16, immSExt16, Int32Regs>;
def MULi    : ArithLogicRRI<0b0001000, "muli",  mul,  simm16, immSExt16, Int32Regs>;
def MULiu   : ArithLogicRRI<0b0001010, "muliu", mul,  uimm16, immZExt16, Int32Regs>;
def DIVi    : ArithLogicRRI<0b0001100, "divi",  sdiv, simm16, immSExt16, Int32Regs>;
def DIViu   : ArithLogicRRI<0b0001101, "diviu", udiv, uimm16, immZExt16, Int32Regs>;
def ANDi    : ArithLogicRRI<0b0010100, "andi",  and,  uimm16, immZExt16, Int32Regs>;
def ORi     : ArithLogicRRI<0b0010110, "ori",   or,   uimm16, immZExt16, Int32Regs>;

def ADD     : ArithLogicRRR<0b0000001, "add",  add,  Int32Regs>;
def ADDu    : ArithLogicRRR<0b0000011, "addu", add,  Int32Regs>;
def SUB     : ArithLogicRRR<0b0000101, "sub",  sub,  Int32Regs>;
def MUL     : ArithLogicRRR<0b0000111, "mul",  mul,  Int32Regs>;
def MULu    : ArithLogicRRR<0b0001001, "mulu", mul,  Int32Regs>;
def DIV     : ArithLogicRRR<0b0001011, "div",  sdiv, Int32Regs>;
def DIVu    : ArithLogicRRR<0b0001101, "divu", udiv, Int32Regs>;
def MOD     : ArithLogicRRR<0b0001111, "mod",  srem, Int32Regs>;
def MODu    : ArithLogicRRR<0b0010001, "modu", urem, Int32Regs>;

def AND     : ArithLogicRRR<0b0010011, "and", and, Int32Regs>;
def XOR     : ArithLogicRRR<0b0010111, "xor", xor, Int32Regs>;
def OR      : ArithLogicRRR<0b0010101, "or",  or, Int32Regs>;

def SHL     : ArithLogicRRR<0b0011001, "shl", shl, Int32Regs>;
def SAR     : ArithLogicRRR<0b0011101, "sar", sra, Int32Regs>;
def SLR     : ArithLogicRRR<0b0011011, "slr", srl, Int32Regs>;

def EQ      : CompareRRR<0b0011111, "eq", seteq, Int32Regs>;
def LT      : CompareRRR<0b0100011, "lt", setlt, Int32Regs>;
def GT      : CompareRRR<0b0100111, "gt", setgt, Int32Regs>;
def LE      : CompareRRR<0b0101011, "le", setle, Int32Regs>;
def GE      : CompareRRR<0b0101111, "ge", setge, Int32Regs>;
def LTU     : CompareRRR<0b0100101, "ltu",setult,Int32Regs>;
def GTU     : CompareRRR<0b0101001, "gtu",setugt,Int32Regs>;

/* 数据传输 */
defm LW     : LoadM32<0b1100011,  "lw",  load_a>;
defm SW     : StoreM32<0b1110000, "sw",  store_a>;
defm LEAW   : LeaM32<0b0000100, "add">;

def LI      : InstrRI<0b1101011, (outs Int32Regs:$ra), (ins simm16:$imm16),
            "li\t$ra, $imm16",
            [(set Int32Regs:$ra, (i32 immSExt16:$imm16))], NoItinerary>;
def LIH     : InstrRI<0b1101101, (outs Int32Regs:$ra), (ins simm16:$imm16),
            "lih\t$ra, $imm16",
            [(set Int32Regs:$ra, (i32 immSExt16:$imm16))], NoItinerary>;

def PUSH    : InstrR<0b1111001, (outs), (ins Int32Regs:$ra), "push\t$ra",
            [(AlexPush Int32Regs:$ra)], NoItinerary>;
def POP     : InstrR<0b1111010, (outs Int32Regs:$ra), (ins), "pop\t$ra",
            [(set Int32Regs:$ra, (AlexPop))], NoItinerary>;

def MTRA    : InstrRI<0b1111101, (outs), (ins Int32Regs:$ra, simm16:$imm16),
            "mtra\t$imm16($ra)", [], NoItinerary>;
def MFRA    : InstrR<0b1111110, (outs Int32Regs:$ra), (ins),
            "mfra\t$ra", [], NoItinerary>;
def RET     : InstrPlain<0b1111110, "ret", [], NoItinerary>;


/* Global Address/Jump table */

/*def : Pat<(AlexHi tjumptable:$in), (LIH tjumptable:$in)>;
def : Pat<(AlexLo tjumptable:$in), (LI tjumptable:$in)>;
def : Pat<(add Int32Regs:$hi, (AlexLo tjumptable:$lo)),
              (ORi Int32Regs:$hi, tjumptable:$lo)>;
*/
def AlexWrapper    : SDNode<"AlexISD::Wrapper", SDTIntUnaryOp>;
class WrapperPat<SDNode node, RegisterClass RC>:
      Pat<(AlexWrapper node:$in),
              (LI node:$in)>;

def : WrapperPat<tglobaladdr, Int32Regs>;
//def : WrapperPat<tjumptable,  ORi, Int32Regs>;

/* 分支语句 */
//def JR      : JumpFR<0b1011111, "jr", Int32Regs>;
def JRRA    : InstrR<0b1100010, (outs), (ins Int32Regs:$ra), "jrra\t$ra", [], NoItinerary>;

let isReturn=1, isTerminator=1, hasDelaySlot=1, isBarrier=1, hasCtrlDep=1 in
    def RetLR : AlexPseudo<123, (outs), (ins), "", [(AlexRet)]>;

def RETB     : RetBase<Int32Regs>;

class CBranch24<bits<7> op, string instr_asm, RegisterClass RC,
                   list<Register> UseRegs>:
  InstrI<op, (outs), (ins RC:$ra, bb:$imm16),
             !strconcat(instr_asm, "\t$ra, $imm16"),
             [], NoItinerary> {
  let isBranch = 1;
  let isTerminator = 1;
  let hasDelaySlot = 1;
}
def brtarget24    : Operand<OtherVT> {
  let EncoderMethod = "getBranch24TargetOpValue";
  let OperandType = "OPERAND_PCREL";
  let DecoderMethod = "DecodeBranch24Target";
}

def JR : InstrI<0b1011111, (outs), (ins brtarget24:$imm16),
             "jr\t$imm16", [(br bb:$imm16)], NoItinerary> {
  let isBranch = 1;
  let isTerminator = 1;
  let isBarrier = 1;
  let hasDelaySlot = 1;
}
def BNZ : InstrRI<0b1010010, (outs), (ins Int32Regs:$ra, brtarget24:$imm16),
          "bnz\t$ra, $imm16",
          [(brcond Int32Regs:$ra, bb:$imm16)], NoItinerary> {
  let isBranch = 1;
  let isTerminator = 1;
  let isBarrier = 1;
  let hasDelaySlot = 1;
}
def BNE : InstrRRI<0b1010110, (outs), (ins Int32Regs:$ra, Int32Regs:$rb, brtarget24:$imm16),
        "bne\t$ra, $rb, $imm16",
         [(brcond (i32 (setne Int32Regs:$ra, Int32Regs:$rb)), bb:$imm16)], NoItinerary> {
  let isBranch = 1;
  let isTerminator = 1;
  let isBarrier = 1;
  let hasDelaySlot = 1;
}

def BLT : InstrRRI<0b1011000, (outs), (ins Int32Regs:$ra, Int32Regs:$rb, brtarget24:$imm16),
        "blt\t$ra, $rb, $imm16",
         [(brcond (i32 (setlt Int32Regs:$ra, Int32Regs:$rb)), bb:$imm16)], NoItinerary> {
  let isBranch = 1;
  let isTerminator = 1;
  let isBarrier = 1;
  let hasDelaySlot = 1;
}

def BGT : InstrRRI<0b1011011, (outs), (ins Int32Regs:$ra, Int32Regs:$rb, brtarget24:$imm16),
        "bgt\t$ra, $rb, $imm16",
         [(brcond (i32 (setgt Int32Regs:$ra, Int32Regs:$rb)), bb:$imm16)], NoItinerary> {
  let isBranch = 1;
  let isTerminator = 1;
  let isBarrier = 1;
  let hasDelaySlot = 1;
}


/* 函数调用 */
def calltarget  : Operand<iPTR> {
  let EncoderMethod = "getJumpTargetOpValue";
}

def SDT_AlexCallLink      : SDTypeProfile<0, 1, [SDTCisVT<0, iPTR>]>;
def AlexCallLink : SDNode<"AlexISD::JmpLink",SDT_AlexCallLink,
                         [SDNPHasChain, SDNPOutGlue, SDNPOptInGlue,
                          SDNPVariadic]>;

let isCall=1, hasDelaySlot=1 in {
  class CallLink<bits<7> op, string instr_asm>:
    InstrI<op, (outs), (ins calltarget:$imm16, variable_ops),
       !strconcat(instr_asm, "\t$imm16"), [/*(AlexCallLink imm:$imm16)*/],
       NoItinerary>;

  class CallLinkReg<bits<7> op, string instr_asm,
                    RegisterClass RC>:
    InstrR<op, (outs), (ins RC:$ra, variable_ops),
       !strconcat(instr_asm, "\t$ra"), [/*(AlexCallLink RC:$ra)*/], NoItinerary>;
}

def CALL    : CallLink<0b1100010, "call">;

//def : Pat<(AlexCallLink (i32 tglobaladdr:$dst)),
//          (J tglobaladdr:$dst)>;
//def : Pat<(AlexCallLink (i32 texternalsym:$dst)),
//          (J texternalsym:$dst)>;

let isTerminator=1, hasDelaySlot=1, isBarrier=1, hasCtrlDep=1 in
    def CALLg : AlexPseudo<0, (outs), (ins calltarget:$imm16), "",
     [(AlexCallLink (i32 tglobaladdr:$imm16))]>;
let isTerminator=1, hasDelaySlot=1, isBarrier=1, hasCtrlDep=1 in
    def CALLi : AlexPseudo<0, (outs), (ins calltarget:$imm16), "",
     [(AlexCallLink (i32 imm:$imm16))]>;
let isTerminator=1, hasDelaySlot=1, isBarrier=1, hasCtrlDep=1 in
    def CALLr : AlexPseudo<0, (outs), (ins Int32Regs:$ra), "",
     [(AlexCallLink Int32Regs:$ra)]>;


def LI32 : AlexPseudo<0, (outs Int32Regs:$ra), (ins simm32:$imm), "",
    [(set Int32Regs:$ra, (AlexLI32 immSExt32:$imm))]>;

def SDT_AlexCallSeqStart : SDCallSeqStart<[SDTCisVT<0, i32>]>;
def SDT_AlexCallSeqEnd   : SDCallSeqEnd<[SDTCisVT<0, i32>, SDTCisVT<1, i32>]>;
def callseq_start : SDNode<"ISD::CALLSEQ_START", SDT_AlexCallSeqStart,
                           [SDNPHasChain, SDNPOutGlue]>;
def callseq_end   : SDNode<"ISD::CALLSEQ_END", SDT_AlexCallSeqEnd,
                           [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;

def ADJCALLSTACKDOWN : AlexPseudo<124, (outs), (ins uimm16:$amt),
                                  "!ADJCALLSTACKDOWN $amt",
                                  [(callseq_start timm:$amt)]>;
def ADJCALLSTACKUP   : AlexPseudo<125, (outs), (ins uimm16:$amt1, uimm16:$amt2),
                                  "!ADJCALLSTACKUP $amt1",
                                  [(callseq_end timm:$amt1, timm:$amt2)]>;

// 模式匹配
def : Pat<(i32 immSExt16:$in),
          (LI imm:$in)>;
def : Pat<(i32 addr:$in),
          (LW mem:$in)>;

def : Pat<(i32 immSExt32:$in),
          (LI32 imm:$in)>;


//def : Pat<(frameindex),
//          (LI 0)>;
//def addr :
//  ComplexPattern<iPTR, 2, "SelectAddr", [frameindex], [SDNPWantParent]>;
