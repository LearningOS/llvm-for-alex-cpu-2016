// Generic Alex Format
class AlexInst<dag outs, dag ins, string asmstr, list<dag> pattern,
               InstrItinClass itin>: Instruction
{
  field bits<32> Inst;

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

  // TSFlags layout should be kept in sync with AlexInstrInfo.h.
  //let TSFlags{3-0}   = FormBits;

  let DecoderNamespace = "Alex";

  field bits<32> SoftFail = 0;
}

// Alex Pseudo Instructions Format
class AlexPseudo<dag outs, dag ins, string asmstr, list<dag> pattern = []>:
      AlexInst<outs, ins, asmstr, pattern, NoItinerary> {
  let isCodeGenOnly = 1;
  let isPseudo = 1;
}

class InstrPlain<bits<8> op, string asmstr, list<dag> pattern = [],
         InstrItinClass itin = NoItinerary>: AlexInst<(outs), (ins), asmstr, pattern, itin>
{
  let Opcode = op;
  let Inst{23-0} = 0;
}
class InstrR<bits<8> op, dag outs, dag ins, string asmstr, list<dag> pattern = [],
         InstrItinClass itin = NoItinerary>: AlexInst<outs, ins, asmstr, pattern, itin>
{
  bits<4>  ra;

  let Opcode = op;

  let Inst{23-20} = ra;
  let Inst{19-0} = 0;
}
class InstrRI<bits<8> op, dag outs, dag ins, string asmstr, list<dag> pattern,
         InstrItinClass itin>: AlexInst<outs, ins, asmstr, pattern, itin>
{
  bits<4>  ra;
  bits<16> imm16;

  let Opcode = op;

  let Inst{23-20} = ra;
  let Inst{19-16} = 0;
  let Inst{15-0}  = imm16;
}
class InstrRR<bits<8> op, dag outs, dag ins, string asmstr, list<dag> pattern = [],
         InstrItinClass itin = NoItinerary>: AlexInst<outs, ins, asmstr, pattern, itin>
{
  bits<4>  ra;
  bits<4>  rb;

  let Opcode = op;

  let Inst{23-20} = ra;
  let Inst{19-16} = rb;
  let Inst{15-0}  = 0;
}
class InstrRRI<bits<8> op, dag outs, dag ins, string asmstr, list<dag> pattern,
         InstrItinClass itin>: AlexInst<outs, ins, asmstr, pattern, itin>
{
  bits<4>  ra;
  bits<4>  rb;
  bits<16> imm16;

  let Opcode = op;

  let Inst{23-20} = ra;
  let Inst{19-16} = rb;
  let Inst{15-0}  = imm16;
}
class InstrRRR<bits<8> op, dag outs, dag ins, string asmstr, list<dag> pattern,
         InstrItinClass itin>: AlexInst<outs, ins, asmstr, pattern, itin>
{
  bits<4>  ra;
  bits<4>  rb;
  bits<4>  rc;

  let Opcode = op;

  let Inst{23-20} = ra;
  let Inst{19-16} = rb;
  let Inst{15-12} = rc;
  let Inst{11-0}  = 0;
}

class InstrI<bits<8> op, dag outs, dag ins, string asmstr, list<dag> pattern,
         InstrItinClass itin>: AlexInst<outs, ins, asmstr, pattern, itin>
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

class AlexAsmPseudoInst<dag outs, dag ins, string asmstr>:
  AlexInst<outs, ins, asmstr, [], NoItinerary> {
  let isPseudo = 1;
  let Pattern = [];
}
def AlexMemAsmOperand : AsmOperandClass {
  let Name = "Mem";
  let ParserMethod = "parseMemOperand";
}

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
  let ParserMatchClass = AlexMemAsmOperand;
}

class LoadImm32< string instr_asm, Operand Od, RegisterClass RC> :
  AlexAsmPseudoInst<(outs RC:$ra), (ins Od:$imm32),
                     !strconcat(instr_asm, "\t$ra, $imm32")> ;
def LoadImm32Reg : LoadImm32<"li", shamt, Int32Regs>;

class LoadAddress<string instr_asm, Operand MemOpnd, RegisterClass RC> :
  AlexAsmPseudoInst<(outs RC:$ra), (ins MemOpnd:$addr),
                     !strconcat(instr_asm, "\t$ra, $addr")> ;
def LoadAddr32Reg : LoadAddress<"la", mem, Int32Regs>;

class LoadAddressImm<string instr_asm, Operand Od, RegisterClass RC> :
  AlexAsmPseudoInst<(outs RC:$ra), (ins Od:$imm32),
                     !strconcat(instr_asm, "\t$ra, $imm32")> ;
def LoadAddr32Imm : LoadAddressImm<"la", shamt, Int32Regs>;

def memaddr : Operand<iPTR> {
  let PrintMethod = "printMemAddrOperand";
  let MIOperandInfo = (ops Int32Regs, simm16);
  let EncoderMethod = "getMemEncoding";
}

def AlexHi    : SDNode<"AlexISD::Hi", SDTIntUnaryOp>;
def AlexLo    : SDNode<"AlexISD::Lo", SDTIntUnaryOp>;

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
//def a

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
//def immZExt5 : ImmLeaf<i32, [{return Imm == (Imm & 0x1f);}]>;

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
def zextloadi16_a   : AlignedLoad<zextloadi16>;
def extloadi16_a    : AlignedLoad<extloadi16>;
def truncstorei16_a : AlignedStore<truncstorei16>;

/* 逻辑算术指令的模板 */
class ArithLogicRR<bits<8> op, string instr_asm, SDNode OpNode,
                  Operand Od, PatLeaf imm_type, RegisterClass RC> :
  InstrRR<op, (outs Int32Regs:$ra), (ins RC:$rb),
     !strconcat(instr_asm, "\t$ra, $rb"),
     [(set Int32Regs:$ra, (OpNode RC:$rb))], NoItinerary> {
  let isReMaterializable = 1;
}

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
class CompareRRR<bits<8> op, string instr_asm, PatFrag CmpNode, RegisterClass RC> :
  InstrRRR<op, (outs Int32Regs:$ra), (ins RC:$rb, RC:$rc),
     !strconcat(instr_asm, "\t$ra, $rb, $rc"),
     [(set RC:$ra, (CmpNode RC:$rb, RC:$rc))], NoItinerary> {
  let isReMaterializable = 1;
}

/* 内存访问指令模板 */
class FMem<bits<8> op, dag outs, dag ins, string asmstr, list<dag> pattern,
          InstrItinClass itin>: InstrRRI<op, outs, ins, asmstr, pattern, itin> {
  bits<20> addr;
  let rb{3-0} = addr{19-16};
  let Inst{15-0}  = addr{15-0};
  let DecoderMethod = "DecodeMem";
}

/* load/store模板 */
// Memory Load/Store
let canFoldAsLoad = 1 in
class LoadM<bits<8> op, string instr_asm, PatFrag OpNode, RegisterClass RC,
            Operand MemOpnd, bit Pseudo>:
  FMem<op, (outs RC:$ra), (ins MemOpnd:$addr),
     !strconcat(instr_asm, "\t$ra, $addr"),
     [(set RC:$ra, (OpNode addr:$addr))], NoItinerary> {
  let isPseudo = Pseudo;
}

class LeaM<bits<8> op, string instr_asm, RegisterClass RC,
            Operand MemOpnd, bit Pseudo>:
  FMem<op, (outs RC:$ra), (ins MemOpnd:$addr),
     !strconcat(instr_asm, "\t$ra, $addr"),
     [(set RC:$ra, addr:$addr)], NoItinerary> {
  let isPseudo = Pseudo;
}

class StoreM<bits<8> op, string instr_asm, PatFrag OpNode, RegisterClass RC,
             Operand MemOpnd, bit Pseudo>:
  FMem<op, (outs), (ins RC:$ra, MemOpnd:$addr),
     !strconcat(instr_asm, "\t$addr, $ra"),
     [(OpNode RC:$ra, addr:$addr)], NoItinerary> {
  let isPseudo = Pseudo;
}

//@ 32-bit load.
multiclass LoadM32<bits<8> op, string instr_asm, PatFrag OpNode,
                   bit Pseudo = 0> {
  def #NAME# : LoadM<op, instr_asm, OpNode, Int32Regs, mem, Pseudo>;
}
multiclass LeaM32<bits<8> op, string instr_asm,
                   bit Pseudo = 0> {
  def #NAME# : LeaM<op, instr_asm, Int32Regs, memaddr, Pseudo>;
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
     !strconcat(instr_asm, "\t$ra"), [], NoItinerary> {
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
def NOP     : InstrPlain<0x00, "nop", [], NoItinerary>;

/* 算数逻辑指令 */

def ADDi    : ArithLogicRRI<0x02, "addi",  add,  simm16, immSExt16, Int32Regs>;
def ADDiu   : ArithLogicRRI<0x03, "addiu", add,  uimm16, immZExt16, Int32Regs>;
def SUBi    : ArithLogicRRI<0x05, "subi",  sub,  simm16, immSExt16, Int32Regs>;
def MULi    : ArithLogicRRI<0x08, "muli",  mul,  simm16, immSExt16, Int32Regs>;
def MULiu   : ArithLogicRRI<0x09, "muliu", mul,  uimm16, immZExt16, Int32Regs>;
def DIVi    : ArithLogicRRI<0x0b, "divi",  sdiv, simm16, immSExt16, Int32Regs>;
def DIViu   : ArithLogicRRI<0x0c, "diviu", udiv, uimm16, immZExt16, Int32Regs>;

// 需要ori
def ORi     : ArithLogicRRI<0x42, "ori",   or,   uimm16, immZExt16, Int32Regs>;

def ADD     : ArithLogicRRR<0x01, "add",  add,  Int32Regs>;
def SUB     : ArithLogicRRR<0x04, "sub",  sub,  Int32Regs>;
def MUL     : ArithLogicRRR<0x07, "mul",  mul,  Int32Regs>;
def DIV     : ArithLogicRRR<0x0a, "div",  sdiv, Int32Regs>;
def DIVu    : ArithLogicRRR<0x43, "divu", udiv, Int32Regs>;
def MOD     : ArithLogicRRR<0x0d, "mod",  srem, Int32Regs>;
def MODu    : ArithLogicRRR<0x44, "modu", urem, Int32Regs>;

def SHL     : ArithLogicRRR<0x10, "shl", shl, Int32Regs>;
def SLR     : ArithLogicRRR<0x12, "slr", srl, Int32Regs>;
def SAR     : ArithLogicRRR<0x14, "sar", sra, Int32Regs>;

def AND     : ArithLogicRRR<0x16, "and", and, Int32Regs>;
def OR      : ArithLogicRRR<0x17, "or",  or, Int32Regs>;
def XOR     : ArithLogicRRR<0x18, "xor", xor, Int32Regs>;

def EQ      : CompareRRR<0x1a, "eq", seteq, Int32Regs>;
def NE      : CompareRRR<0x1b, "ne", setne, Int32Regs>;
def LT      : CompareRRR<0x1c, "lt", setlt, Int32Regs>;
def LTU     : CompareRRR<0x1d, "ltu",setult,Int32Regs>;
def GT      : CompareRRR<0x1e, "gt", setgt, Int32Regs>;
def GTU     : CompareRRR<0x1f, "gtu",setugt,Int32Regs>;
def LE      : CompareRRR<0x20, "le", setle, Int32Regs>;
def LEU     : CompareRRR<0x21, "leu",setule, Int32Regs>;
def GE      : CompareRRR<0x22, "ge", setge, Int32Regs>;
def GEU     : CompareRRR<0x22, "geu",setuge, Int32Regs>;

/* 数据传输 */
/* Load */
defm LW     : LoadM32<0x2d,  "lw",  load_a>;
defm LB     : LoadM32<0x2f,  "lb",  zextloadi8>;
defm LH     : LoadM32<0x2e,  "lh",  zextloadi16_a>;

// 下面的load都是伪指令了
defm LHs    : LoadM32<0xff,  "lhs", sextloadi16, 1>;
defm LHa    : LoadM32<0xff,  "lha", extloadi16, 1>;
defm LBs    : LoadM32<0xff,  "lbs", sextloadi8, 1>;
defm LBa    : LoadM32<0xff,  "lba", extloadi8, 1>;
defm LBIT   : LoadM32<0xff,  "lbit", zextloadi1, 1>;
defm LBITs  : LoadM32<0xff,  "lbits", sextloadi1, 1>;
defm LBITa  : LoadM32<0xff,  "lbita", extloadi1, 1>;

/* Save */
defm SW     : StoreM32<0x34, "sw",  store_a>;
defm SB     : StoreM32<0x36, "sb",  truncstorei8>;
defm SH     : StoreM32<0x08, "sh",  truncstorei16_a>;

// lea伪指令
defm LEAW   : LeaM32<0x02, "addi">;

/* Load Immediate */
def LI      : InstrRI<0x31, (outs Int32Regs:$ra), (ins simm16:$imm16),
            "li\t$ra, $imm16",
            [(set Int32Regs:$ra, (i32 immSExt16:$imm16))], NoItinerary>;
def LIh     : InstrRI<0x33, (outs Int32Regs:$ra), (ins simm16:$imm16),
            "lih\t$ra, $imm16",
            [], NoItinerary>;
// load immediate 32 伪指令
def LI32    : AlexPseudo<(outs Int32Regs:$ra), (ins simm32:$imm), "li32",
            [(set Int32Regs:$ra, (AlexLI32 immSExt32:$imm))]>;

def CombineLoHiRI : AlexPseudo<(outs Int32Regs:$ra),
            (ins Int32Regs:$rb, simm16:$imm),
            "combine_lo_hi_ri\t$ra, $rb, $imm">;

/* 分支语句 */

/* Return Pseudo */
let isReturn=1, isTerminator=1, hasDelaySlot=1, isBarrier=1, hasCtrlDep=1 in
    def RetLR : AlexPseudo<(outs), (ins), "", [(AlexRet)]>;
def RETB     : RetBase<Int32Regs>;
def RET     : InstrPlain<0x2c, "ret", [], NoItinerary>;

def brtarget16    : Operand<OtherVT> {
  let EncoderMethod = "getBranch16TargetOpValue";
  let OperandType = "OPERAND_PCREL";
  let DecoderMethod = "DecodeBranch16Target";
}
let isBranch = 1, isTerminator = 1, isBarrier = 1, hasDelaySlot = 1 in {
def BNZ : InstrRI<0x26, (outs), (ins Int32Regs:$ra, brtarget16:$imm16),
          "bnz\t$ra, $imm16",
          [(brcond Int32Regs:$ra, bb:$imm16)], NoItinerary>;
def BE  : InstrRRI<0x25, (outs), (ins Int32Regs:$ra, Int32Regs:$rb, brtarget16:$imm16),
        "be\t$ra, $rb, $imm16",
         [(brcond (i32 (setle Int32Regs:$ra, Int32Regs:$rb)), bb:$imm16)], NoItinerary>;

def BNE : InstrRRI<0x26, (outs), (ins Int32Regs:$ra, Int32Regs:$rb, brtarget16:$imm16),
        "bne\t$ra, $rb, $imm16",
         [(brcond (i32 (setge Int32Regs:$ra, Int32Regs:$rb)), bb:$imm16)], NoItinerary>;
def BNEZ : InstrRI<0x26, (outs), (ins Int32Regs:$ra, brtarget16:$imm16),
        "bnez\t$ra, $imm16",
         [(brcond Int32Regs:$ra, bb:$imm16)], NoItinerary>;
def BLT : InstrRRI<0x27, (outs), (ins Int32Regs:$ra, Int32Regs:$rb, brtarget16:$imm16),
        "blt\t$ra, $rb, $imm16",
         [(brcond (i32 (setlt Int32Regs:$ra, Int32Regs:$rb)), bb:$imm16)], NoItinerary>;
def BGT : InstrRRI<0x28, (outs), (ins Int32Regs:$ra, Int32Regs:$rb, brtarget16:$imm16),
        "bgt\t$ra, $rb, $imm16",
         [(brcond (i32 (setgt Int32Regs:$ra, Int32Regs:$rb)), bb:$imm16)], NoItinerary>;
}

/* 无条件跳转 */
def jmptarget  : Operand<OtherVT> {
  let EncoderMethod = "getBranch16TargetOpValue";
  let OperandType = "OPERAND_PCREL";
}

let isBranch = 1, isTerminator = 1, isBarrier = 1, hasDelaySlot = 1 in {
    def J : InstrI<0x29, (outs), (ins jmptarget:$imm16),
             "j\t$imm16", [(br bb:$imm16)], NoItinerary>;

}
let isBranch=1, isTerminator=1, isBarrier=1, hasDelaySlot = 1,
    isIndirectBranch = 1 in {
    def JR : InstrR<0x2a, (outs), (ins Int32Regs:$ra), "jr\t$ra", []>;
}

def : Pat<(brind Int32Regs:$ra),
          (JR Int32Regs:$ra)>;

/* 函数调用 */
def calltarget  : Operand<iPTR> {
  let EncoderMethod = "getJumpTargetOpValue";
}
def SDT_AlexCallLink      : SDTypeProfile<0, 1, [SDTCisVT<0, iPTR>]>;
def AlexCallLink : SDNode<"AlexISD::JmpLink",SDT_AlexCallLink,
                         [SDNPHasChain, SDNPOutGlue, SDNPOptInGlue,
                          SDNPVariadic]>;
/* 真正的函数调用指令(只能寄存器寻址) */
let isCall=1, hasDelaySlot=1 in {
  class CallLinkReg<bits<8> op, string instr_asm,
                    RegisterClass RC>:
    InstrR<op, (outs), (ins RC:$ra, variable_ops),
       !strconcat(instr_asm, "\t$ra"), [], NoItinerary>;
}
def CALL    : CallLinkReg<0x2b, "call", Int32Regs>;

let isTerminator=1, hasDelaySlot=1, isBarrier=1, hasCtrlDep=1 in {
    def CALLg : AlexPseudo<(outs), (ins calltarget:$imm16), "",
                [(AlexCallLink (i32 tglobaladdr:$imm16))]>;
    def CALLr : AlexPseudo<(outs), (ins Int32Regs:$ra), "",
                [(AlexCallLink Int32Regs:$ra)]>;
}

def SDT_AlexCallSeqStart : SDCallSeqStart<[SDTCisVT<0, i32>]>;
def SDT_AlexCallSeqEnd   : SDCallSeqEnd<[SDTCisVT<0, i32>, SDTCisVT<1, i32>]>;
def callseq_start : SDNode<"ISD::CALLSEQ_START", SDT_AlexCallSeqStart,
                           [SDNPHasChain, SDNPOutGlue]>;
def callseq_end   : SDNode<"ISD::CALLSEQ_END", SDT_AlexCallSeqEnd,
                           [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;
let Defs = [SP], Uses = [SP] in {
    def ADJCALLSTACKDOWN : AlexPseudo<(outs), (ins uimm16:$amt),
                                      "!ADJCALLSTACKDOWN $amt",
                                      [(callseq_start timm:$amt)]>;
    def ADJCALLSTACKUP   : AlexPseudo<(outs), (ins uimm16:$amt1, uimm16:$amt2),
                                      "!ADJCALLSTACKUP $amt1",
                                      [(callseq_end timm:$amt1, timm:$amt2)]>;
}
/* 系统指令 */
def BIN : InstrRR<0x80, (outs Int32Regs:$ra), (ins Int32Regs:$rb), "bin\t$ra, $rb">;
def BOUT: InstrRR<0x81, (outs), (ins Int32Regs:$ra, Int32Regs:$rb),"bout\t$ra, $rb">;
def HALT: InstrPlain<0, "halt"> {
    let Inst = 0xFFFFFFFF;
}

// 模式匹配
/* 16/32位整数加载的直接翻译 */
def : Pat<(i32 immSExt16:$in),
          (LI imm:$in)>;
def : Pat<(i32 addr:$in),
          (LW mem:$in)>;
def : Pat<(i32 immSExt32:$in),
          (LI32 imm:$in)>;

/* Hi/Lo的加载的直接翻译 */
/* Global Address/Jump table */

def : Pat<(AlexHi tblockaddress:$in), (LIh tblockaddress:$in)>;
def : Pat<(AlexLo tblockaddress:$in), (LI tblockaddress:$in)>;
def : Pat<(add Int32Regs:$lo, (AlexHi tblockaddress:$hi)),
          (CombineLoHiRI Int32Regs:$lo, tblockaddress:$hi)>;

def : Pat<(AlexHi tjumptable:$in), (LIh tjumptable:$in)>;
def : Pat<(AlexLo tjumptable:$in), (LI tjumptable:$in)>;
def : Pat<(add Int32Regs:$lo, (AlexHi tjumptable:$hi)),
          (CombineLoHiRI Int32Regs:$lo, tjumptable:$hi)>;

def : Pat<(AlexHi tglobaladdr:$in), (LIh tglobaladdr:$in)>;
def : Pat<(AlexLo tglobaladdr:$in), (LI tglobaladdr:$in)>;
def : Pat<(add Int32Regs:$lo, (AlexHi tglobaladdr:$hi)),
          (CombineLoHiRI Int32Regs:$lo, tglobaladdr:$hi)>;

def AlexWrapper    : SDNode<"AlexISD::Wrapper", SDTIntUnaryOp>;
class WrapperPat<SDNode node>:
      Pat<(AlexWrapper node:$in),
              (LI32 node:$in)>;

def : WrapperPat<tglobaladdr>;
def : WrapperPat<tjumptable>;

/* 普通有符号比较的分支指令的直接翻译 */
def : Pat<(brcc SETLT, Int32Regs:$ra, Int32Regs:$rb, bb:$imm16),
          (BNEZ (LT Int32Regs:$ra, Int32Regs:$rb), brtarget16:$imm16)>;
def : Pat<(brcc SETGT, Int32Regs:$ra, Int32Regs:$rb, bb:$imm16),
          (BNEZ (GT Int32Regs:$ra, Int32Regs:$rb), brtarget16:$imm16)>;
def : Pat<(brcc SETLE, Int32Regs:$ra, Int32Regs:$rb, bb:$imm16),
          (BNEZ (LE Int32Regs:$ra, Int32Regs:$rb), brtarget16:$imm16)>;
def : Pat<(brcc SETGE, Int32Regs:$ra, Int32Regs:$rb, bb:$imm16),
          (BNEZ (GE Int32Regs:$ra, Int32Regs:$rb), brtarget16:$imm16)>;
def : Pat<(brcc SETEQ, Int32Regs:$ra, Int32Regs:$rb, bb:$imm16),
          (BNEZ (EQ Int32Regs:$ra, Int32Regs:$rb), brtarget16:$imm16)>;
def : Pat<(brcc SETNE, Int32Regs:$ra, Int32Regs:$rb, bb:$imm16),
          (BNEZ (NE Int32Regs:$ra, Int32Regs:$rb), brtarget16:$imm16)>;

/* 无符号分支指令的直接翻译 */
def : Pat<(brcc SETUGT, Int32Regs:$ra, Int32Regs:$rb, bb:$imm16),
          (BNEZ (GTU Int32Regs:$ra, Int32Regs:$rb), brtarget16:$imm16)>;
def : Pat<(brcc SETULT, Int32Regs:$ra, Int32Regs:$rb, bb:$imm16),
          (BNEZ (LTU Int32Regs:$ra, Int32Regs:$rb), brtarget16:$imm16)>;
def : Pat<(brcc SETUGE, Int32Regs:$ra, Int32Regs:$rb, bb:$imm16),
          (BNEZ (GEU Int32Regs:$ra, Int32Regs:$rb), brtarget16:$imm16)>;
def : Pat<(brcc SETULE, Int32Regs:$ra, Int32Regs:$rb, bb:$imm16),
          (BNEZ (LEU Int32Regs:$ra, Int32Regs:$rb), brtarget16:$imm16)>;

/* "?:"操作符会生成select指令 */
def SELECT : AlexPseudo<(outs Int32Regs:$rv),
                        (ins Int32Regs:$ra, Int32Regs:$rb, Int32Regs:$rc),
                        "select",
                        [(set Int32Regs:$rv, (select Int32Regs:$ra, Int32Regs:$rb, Int32Regs:$rc))]>;

/* 多种加法的直接翻译 */
def : Pat<(adde Int32Regs:$ra, immSExt16:$imm),
          (ADDi Int32Regs:$ra, simm16:$imm)>;
def : Pat<(addc Int32Regs:$ra, immSExt16:$imm),
          (ADDi Int32Regs:$ra, simm16:$imm)>;

/* 符号扩展伪指令 */
def SEXT_INREG_1  : AlexPseudo<(outs Int32Regs:$rb), (ins Int32Regs:$ra), "sext_inreg\t$rb, $ra, 1",
        [(set Int32Regs:$rb, (sext_inreg Int32Regs:$ra, i1))]>;
def SEXT_INREG_8  : AlexPseudo<(outs Int32Regs:$rb), (ins Int32Regs:$ra), "sext_inreg\t$rb, $ra, 8",
        [(set Int32Regs:$rb, (sext_inreg Int32Regs:$ra, i8))]>;
def SEXT_INREG_16  : AlexPseudo<(outs Int32Regs:$rb), (ins Int32Regs:$ra), "sext_inreg\t$rb, $ra, 16",
        [(set Int32Regs:$rb, (sext_inreg Int32Regs:$ra, i16))]>;

/* MULHS的错误版本.. */
def MULhu     : ArithLogicRRR<0x07, "mulhu",  mulhu,  Int32Regs>;
def MULhs     : ArithLogicRRR<0x07, "mulhs",  mulhs,  Int32Regs>;