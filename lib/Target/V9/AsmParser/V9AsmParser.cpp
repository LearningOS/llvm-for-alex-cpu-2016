//===-- V9AsmParser.cpp - Parse V9 assembly to MCInst instructions --===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/V9MCExpr.h"
#include "MCTargetDesc/V9MCTargetDesc.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCObjectFileInfo.h"
#include "llvm/MC/MCParser/MCParsedAsmOperand.h"
#include "llvm/MC/MCParser/MCTargetAsmParser.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

// The generated AsmMatcher V9GenAsmMatcher uses "V9" as the target
// namespace. But SPARC backend uses "SP" as its namespace.
namespace llvm {
  namespace V9 {
    using namespace SP;
  }
}

namespace {
class V9Operand;
class V9AsmParser : public MCTargetAsmParser {

  MCAsmParser &Parser;

  /// @name Auto-generated Match Functions
  /// {

#define GET_ASSEMBLER_HEADER
#include "V9GenAsmMatcher.inc"

  /// }

  // public interface of the MCTargetAsmParser.
  bool MatchAndEmitInstruction(SMLoc IDLoc, unsigned &Opcode,
                               OperandVector &Operands, MCStreamer &Out,
                               uint64_t &ErrorInfo,
                               bool MatchingInlineAsm) override;
  bool ParseRegister(unsigned &RegNo, SMLoc &StartLoc, SMLoc &EndLoc) override;
  bool ParseInstruction(ParseInstructionInfo &Info, StringRef Name,
                        SMLoc NameLoc, OperandVector &Operands) override;
  bool ParseDirective(AsmToken DirectiveID) override;

  unsigned validateTargetOperandClass(MCParsedAsmOperand &Op,
                                      unsigned Kind) override;

  // Custom parse functions for V9 specific operands.
  OperandMatchResultTy parseMEMOperand(OperandVector &Operands);

  OperandMatchResultTy parseOperand(OperandVector &Operands, StringRef Name);

  OperandMatchResultTy
  parseV9AsmOperand(std::unique_ptr<V9Operand> &Operand,
                       bool isCall = false);

  OperandMatchResultTy parseBranchModifiers(OperandVector &Operands);

  // Helper function for dealing with %lo / %hi in PIC mode.
  const V9MCExpr *adjustPICRelocation(V9MCExpr::VariantKind VK,
                                         const MCExpr *subExpr);

  // returns true if Tok is matched to a register and returns register in RegNo.
  bool matchRegisterName(const AsmToken &Tok, unsigned &RegNo,
                         unsigned &RegKind);

  bool matchV9AsmModifiers(const MCExpr *&EVal, SMLoc &EndLoc);
  bool parseDirectiveWord(unsigned Size, SMLoc L);

  bool is64Bit() const {
    return getSTI().getTargetTriple().getArch() == Triple::sparcv9;
  }

  void expandSET(MCInst &Inst, SMLoc IDLoc,
                 SmallVectorImpl<MCInst> &Instructions);

public:
  V9AsmParser(const MCSubtargetInfo &sti, MCAsmParser &parser,
                const MCInstrInfo &MII,
                const MCTargetOptions &Options)
      : MCTargetAsmParser(Options, sti), Parser(parser) {
    // Initialize the set of available features.
    setAvailableFeatures(ComputeAvailableFeatures(getSTI().getFeatureBits()));
  }

};

  static const MCPhysReg IntRegs[32] = {
    V9::G0, V9::G1, V9::G2, V9::G3,
    V9::G4, V9::G5, V9::G6, V9::G7,
    V9::O0, V9::O1, V9::O2, V9::O3,
    V9::O4, V9::O5, V9::O6, V9::O7,
    V9::L0, V9::L1, V9::L2, V9::L3,
    V9::L4, V9::L5, V9::L6, V9::L7,
    V9::I0, V9::I1, V9::I2, V9::I3,
    V9::I4, V9::I5, V9::I6, V9::I7 };

  static const MCPhysReg FloatRegs[32] = {
    V9::F0,  V9::F1,  V9::F2,  V9::F3,
    V9::F4,  V9::F5,  V9::F6,  V9::F7,
    V9::F8,  V9::F9,  V9::F10, V9::F11,
    V9::F12, V9::F13, V9::F14, V9::F15,
    V9::F16, V9::F17, V9::F18, V9::F19,
    V9::F20, V9::F21, V9::F22, V9::F23,
    V9::F24, V9::F25, V9::F26, V9::F27,
    V9::F28, V9::F29, V9::F30, V9::F31 };

  static const MCPhysReg DoubleRegs[32] = {
    V9::D0,  V9::D1,  V9::D2,  V9::D3,
    V9::D4,  V9::D5,  V9::D6,  V9::D7,
    V9::D8,  V9::D7,  V9::D8,  V9::D9,
    V9::D12, V9::D13, V9::D14, V9::D15,
    V9::D16, V9::D17, V9::D18, V9::D19,
    V9::D20, V9::D21, V9::D22, V9::D23,
    V9::D24, V9::D25, V9::D26, V9::D27,
    V9::D28, V9::D29, V9::D30, V9::D31 };

  static const MCPhysReg QuadFPRegs[32] = {
    V9::Q0,  V9::Q1,  V9::Q2,  V9::Q3,
    V9::Q4,  V9::Q5,  V9::Q6,  V9::Q7,
    V9::Q8,  V9::Q9,  V9::Q10, V9::Q11,
    V9::Q12, V9::Q13, V9::Q14, V9::Q15 };

  static const MCPhysReg ASRRegs[32] = {
    SP::Y,     SP::ASR1,  SP::ASR2,  SP::ASR3,
    SP::ASR4,  SP::ASR5,  SP::ASR6, SP::ASR7,
    SP::ASR8,  SP::ASR9,  SP::ASR10, SP::ASR11,
    SP::ASR12, SP::ASR13, SP::ASR14, SP::ASR15,
    SP::ASR16, SP::ASR17, SP::ASR18, SP::ASR19,
    SP::ASR20, SP::ASR21, SP::ASR22, SP::ASR23,
    SP::ASR24, SP::ASR25, SP::ASR26, SP::ASR27,
    SP::ASR28, SP::ASR29, SP::ASR30, SP::ASR31};

  static const MCPhysReg IntPairRegs[] = {
    V9::G0_G1, V9::G2_G3, V9::G4_G5, V9::G6_G7,
    V9::O0_O1, V9::O2_O3, V9::O4_O5, V9::O6_O7,
    V9::L0_L1, V9::L2_L3, V9::L4_L5, V9::L6_L7,
    V9::I0_I1, V9::I2_I3, V9::I4_I5, V9::I6_I7};

  static const MCPhysReg CoprocRegs[32] = {
    V9::C0,  V9::C1,  V9::C2,  V9::C3,
    V9::C4,  V9::C5,  V9::C6,  V9::C7,
    V9::C8,  V9::C9,  V9::C10, V9::C11,
    V9::C12, V9::C13, V9::C14, V9::C15,
    V9::C16, V9::C17, V9::C18, V9::C19,
    V9::C20, V9::C21, V9::C22, V9::C23,
    V9::C24, V9::C25, V9::C26, V9::C27,
    V9::C28, V9::C29, V9::C30, V9::C31 };

  static const MCPhysReg CoprocPairRegs[] = {
    V9::C0_C1,   V9::C2_C3,   V9::C4_C5,   V9::C6_C7,
    V9::C8_C9,   V9::C10_C11, V9::C12_C13, V9::C14_C15,
    V9::C16_C17, V9::C18_C19, V9::C20_C21, V9::C22_C23,
    V9::C24_C25, V9::C26_C27, V9::C28_C29, V9::C30_C31};
  
/// V9Operand - Instances of this class represent a parsed V9 machine
/// instruction.
class V9Operand : public MCParsedAsmOperand {
public:
  enum RegisterKind {
    rk_None,
    rk_IntReg,
    rk_IntPairReg,
    rk_FloatReg,
    rk_DoubleReg,
    rk_QuadReg,
    rk_CoprocReg,
    rk_CoprocPairReg,
    rk_Special,
  };

private:
  enum KindTy {
    k_Token,
    k_Register,
    k_Immediate,
    k_MemoryReg,
    k_MemoryImm
  } Kind;

  SMLoc StartLoc, EndLoc;

  struct Token {
    const char *Data;
    unsigned Length;
  };

  struct RegOp {
    unsigned RegNum;
    RegisterKind Kind;
  };

  struct ImmOp {
    const MCExpr *Val;
  };

  struct MemOp {
    unsigned Base;
    unsigned OffsetReg;
    const MCExpr *Off;
  };

  union {
    struct Token Tok;
    struct RegOp Reg;
    struct ImmOp Imm;
    struct MemOp Mem;
  };
public:
  V9Operand(KindTy K) : MCParsedAsmOperand(), Kind(K) {}

  bool isToken() const override { return Kind == k_Token; }
  bool isReg() const override { return Kind == k_Register; }
  bool isImm() const override { return Kind == k_Immediate; }
  bool isMem() const override { return isMEMrr() || isMEMri(); }
  bool isMEMrr() const { return Kind == k_MemoryReg; }
  bool isMEMri() const { return Kind == k_MemoryImm; }

  bool isIntReg() const {
    return (Kind == k_Register && Reg.Kind == rk_IntReg);
  }

  bool isFloatReg() const {
    return (Kind == k_Register && Reg.Kind == rk_FloatReg);
  }

  bool isFloatOrDoubleReg() const {
    return (Kind == k_Register && (Reg.Kind == rk_FloatReg
                                   || Reg.Kind == rk_DoubleReg));
  }

  bool isCoprocReg() const {
    return (Kind == k_Register && Reg.Kind == rk_CoprocReg);
  }

  StringRef getToken() const {
    assert(Kind == k_Token && "Invalid access!");
    return StringRef(Tok.Data, Tok.Length);
  }

  unsigned getReg() const override {
    assert((Kind == k_Register) && "Invalid access!");
    return Reg.RegNum;
  }

  const MCExpr *getImm() const {
    assert((Kind == k_Immediate) && "Invalid access!");
    return Imm.Val;
  }

  unsigned getMemBase() const {
    assert((Kind == k_MemoryReg || Kind == k_MemoryImm) && "Invalid access!");
    return Mem.Base;
  }

  unsigned getMemOffsetReg() const {
    assert((Kind == k_MemoryReg) && "Invalid access!");
    return Mem.OffsetReg;
  }

  const MCExpr *getMemOff() const {
    assert((Kind == k_MemoryImm) && "Invalid access!");
    return Mem.Off;
  }

  /// getStartLoc - Get the location of the first token of this operand.
  SMLoc getStartLoc() const override {
    return StartLoc;
  }
  /// getEndLoc - Get the location of the last token of this operand.
  SMLoc getEndLoc() const override {
    return EndLoc;
  }

  void print(raw_ostream &OS) const override {
    switch (Kind) {
    case k_Token:     OS << "Token: " << getToken() << "\n"; break;
    case k_Register:  OS << "Reg: #" << getReg() << "\n"; break;
    case k_Immediate: OS << "Imm: " << getImm() << "\n"; break;
    case k_MemoryReg: OS << "Mem: " << getMemBase() << "+"
                         << getMemOffsetReg() << "\n"; break;
    case k_MemoryImm: assert(getMemOff() != nullptr);
      OS << "Mem: " << getMemBase()
         << "+" << *getMemOff()
         << "\n"; break;
    }
  }

  void addRegOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");
    Inst.addOperand(MCOperand::createReg(getReg()));
  }

  void addImmOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");
    const MCExpr *Expr = getImm();
    addExpr(Inst, Expr);
  }

  void addExpr(MCInst &Inst, const MCExpr *Expr) const{
    // Add as immediate when possible.  Null MCExpr = 0.
    if (!Expr)
      Inst.addOperand(MCOperand::createImm(0));
    else if (const MCConstantExpr *CE = dyn_cast<MCConstantExpr>(Expr))
      Inst.addOperand(MCOperand::createImm(CE->getValue()));
    else
      Inst.addOperand(MCOperand::createExpr(Expr));
  }

  void addMEMrrOperands(MCInst &Inst, unsigned N) const {
    assert(N == 2 && "Invalid number of operands!");

    Inst.addOperand(MCOperand::createReg(getMemBase()));

    assert(getMemOffsetReg() != 0 && "Invalid offset");
    Inst.addOperand(MCOperand::createReg(getMemOffsetReg()));
  }

  void addMEMriOperands(MCInst &Inst, unsigned N) const {
    assert(N == 2 && "Invalid number of operands!");

    Inst.addOperand(MCOperand::createReg(getMemBase()));

    const MCExpr *Expr = getMemOff();
    addExpr(Inst, Expr);
  }

  static std::unique_ptr<V9Operand> CreateToken(StringRef Str, SMLoc S) {
    auto Op = make_unique<V9Operand>(k_Token);
    Op->Tok.Data = Str.data();
    Op->Tok.Length = Str.size();
    Op->StartLoc = S;
    Op->EndLoc = S;
    return Op;
  }

  static std::unique_ptr<V9Operand> CreateReg(unsigned RegNum, unsigned Kind,
                                                 SMLoc S, SMLoc E) {
    auto Op = make_unique<V9Operand>(k_Register);
    Op->Reg.RegNum = RegNum;
    Op->Reg.Kind   = (V9Operand::RegisterKind)Kind;
    Op->StartLoc = S;
    Op->EndLoc = E;
    return Op;
  }

  static std::unique_ptr<V9Operand> CreateImm(const MCExpr *Val, SMLoc S,
                                                 SMLoc E) {
    auto Op = make_unique<V9Operand>(k_Immediate);
    Op->Imm.Val = Val;
    Op->StartLoc = S;
    Op->EndLoc = E;
    return Op;
  }

  static bool MorphToIntPairReg(V9Operand &Op) {
    unsigned Reg = Op.getReg();
    assert(Op.Reg.Kind == rk_IntReg);
    unsigned regIdx = 32;
    if (Reg >= V9::G0 && Reg <= V9::G7)
      regIdx = Reg - V9::G0;
    else if (Reg >= V9::O0 && Reg <= V9::O7)
      regIdx = Reg - V9::O0 + 8;
    else if (Reg >= V9::L0 && Reg <= V9::L7)
      regIdx = Reg - V9::L0 + 16;
    else if (Reg >= V9::I0 && Reg <= V9::I7)
      regIdx = Reg - V9::I0 + 24;
    if (regIdx % 2 || regIdx > 31)
      return false;
    Op.Reg.RegNum = IntPairRegs[regIdx / 2];
    Op.Reg.Kind = rk_IntPairReg;
    return true;
  }

  static bool MorphToDoubleReg(V9Operand &Op) {
    unsigned Reg = Op.getReg();
    assert(Op.Reg.Kind == rk_FloatReg);
    unsigned regIdx = Reg - V9::F0;
    if (regIdx % 2 || regIdx > 31)
      return false;
    Op.Reg.RegNum = DoubleRegs[regIdx / 2];
    Op.Reg.Kind = rk_DoubleReg;
    return true;
  }

  static bool MorphToQuadReg(V9Operand &Op) {
    unsigned Reg = Op.getReg();
    unsigned regIdx = 0;
    switch (Op.Reg.Kind) {
    default: llvm_unreachable("Unexpected register kind!");
    case rk_FloatReg:
      regIdx = Reg - V9::F0;
      if (regIdx % 4 || regIdx > 31)
        return false;
      Reg = QuadFPRegs[regIdx / 4];
      break;
    case rk_DoubleReg:
      regIdx =  Reg - V9::D0;
      if (regIdx % 2 || regIdx > 31)
        return false;
      Reg = QuadFPRegs[regIdx / 2];
      break;
    }
    Op.Reg.RegNum = Reg;
    Op.Reg.Kind = rk_QuadReg;
    return true;
  }

  static bool MorphToCoprocPairReg(V9Operand &Op) {
    unsigned Reg = Op.getReg();
    assert(Op.Reg.Kind == rk_CoprocReg);
    unsigned regIdx = 32;
    if (Reg >= V9::C0 && Reg <= V9::C31)
      regIdx = Reg - V9::C0;
    if (regIdx % 2 || regIdx > 31)
      return false;
    Op.Reg.RegNum = CoprocPairRegs[regIdx / 2];
    Op.Reg.Kind = rk_CoprocPairReg;
    return true;
  }
  
  static std::unique_ptr<V9Operand>
  MorphToMEMrr(unsigned Base, std::unique_ptr<V9Operand> Op) {
    unsigned offsetReg = Op->getReg();
    Op->Kind = k_MemoryReg;
    Op->Mem.Base = Base;
    Op->Mem.OffsetReg = offsetReg;
    Op->Mem.Off = nullptr;
    return Op;
  }

  static std::unique_ptr<V9Operand>
  CreateMEMr(unsigned Base, SMLoc S, SMLoc E) {
    auto Op = make_unique<V9Operand>(k_MemoryReg);
    Op->Mem.Base = Base;
    Op->Mem.OffsetReg = V9::G0;  // always 0
    Op->Mem.Off = nullptr;
    Op->StartLoc = S;
    Op->EndLoc = E;
    return Op;
  }

  static std::unique_ptr<V9Operand>
  MorphToMEMri(unsigned Base, std::unique_ptr<V9Operand> Op) {
    const MCExpr *Imm  = Op->getImm();
    Op->Kind = k_MemoryImm;
    Op->Mem.Base = Base;
    Op->Mem.OffsetReg = 0;
    Op->Mem.Off = Imm;
    return Op;
  }
};

} // end namespace

void V9AsmParser::expandSET(MCInst &Inst, SMLoc IDLoc,
                               SmallVectorImpl<MCInst> &Instructions) {
  MCOperand MCRegOp = Inst.getOperand(0);
  MCOperand MCValOp = Inst.getOperand(1);
  assert(MCRegOp.isReg());
  assert(MCValOp.isImm() || MCValOp.isExpr());

  // the imm operand can be either an expression or an immediate.
  bool IsImm = Inst.getOperand(1).isImm();
  int64_t RawImmValue = IsImm ? MCValOp.getImm() : 0;

  // Allow either a signed or unsigned 32-bit immediate.
  if (RawImmValue < -2147483648LL || RawImmValue > 4294967295LL) {
    Error(IDLoc, "set: argument must be between -2147483648 and 4294967295");
    return;
  }

  // If the value was expressed as a large unsigned number, that's ok.
  // We want to see if it "looks like" a small signed number.
  int32_t ImmValue = RawImmValue;
  // For 'set' you can't use 'or' with a negative operand on V9 because
  // that would splat the sign bit across the upper half of the destination
  // register, whereas 'set' is defined to zero the high 32 bits.
  bool IsEffectivelyImm13 =
      IsImm && ((is64Bit() ? 0 : -4096) <= ImmValue && ImmValue < 4096);
  const MCExpr *ValExpr;
  if (IsImm)
    ValExpr = MCConstantExpr::create(ImmValue, getContext());
  else
    ValExpr = MCValOp.getExpr();

  MCOperand PrevReg = MCOperand::createReg(V9::G0);

  // If not just a signed imm13 value, then either we use a 'sethi' with a
  // following 'or', or a 'sethi' by itself if there are no more 1 bits.
  // In either case, start with the 'sethi'.
  if (!IsEffectivelyImm13) {
    MCInst TmpInst;
    const MCExpr *Expr = adjustPICRelocation(V9MCExpr::VK_V9_HI, ValExpr);
    TmpInst.setLoc(IDLoc);
    TmpInst.setOpcode(SP::SETHIi);
    TmpInst.addOperand(MCRegOp);
    TmpInst.addOperand(MCOperand::createExpr(Expr));
    Instructions.push_back(TmpInst);
    PrevReg = MCRegOp;
  }

  // The low bits require touching in 3 cases:
  // * A non-immediate value will always require both instructions.
  // * An effectively imm13 value needs only an 'or' instruction.
  // * Otherwise, an immediate that is not effectively imm13 requires the
  //   'or' only if bits remain after clearing the 22 bits that 'sethi' set.
  // If the low bits are known zeros, there's nothing to do.
  // In the second case, and only in that case, must we NOT clear
  // bits of the immediate value via the %lo() assembler function.
  // Note also, the 'or' instruction doesn't mind a large value in the case
  // where the operand to 'set' was 0xFFFFFzzz - it does exactly what you mean.
  if (!IsImm || IsEffectivelyImm13 || (ImmValue & 0x3ff)) {
    MCInst TmpInst;
    const MCExpr *Expr;
    if (IsEffectivelyImm13)
      Expr = ValExpr;
    else
      Expr = adjustPICRelocation(V9MCExpr::VK_V9_LO, ValExpr);
    TmpInst.setLoc(IDLoc);
    TmpInst.setOpcode(SP::ORri);
    TmpInst.addOperand(MCRegOp);
    TmpInst.addOperand(PrevReg);
    TmpInst.addOperand(MCOperand::createExpr(Expr));
    Instructions.push_back(TmpInst);
  }
}

bool V9AsmParser::MatchAndEmitInstruction(SMLoc IDLoc, unsigned &Opcode,
                                             OperandVector &Operands,
                                             MCStreamer &Out,
                                             uint64_t &ErrorInfo,
                                             bool MatchingInlineAsm) {
  MCInst Inst;
  SmallVector<MCInst, 8> Instructions;
  unsigned MatchResult = MatchInstructionImpl(Operands, Inst, ErrorInfo,
                                              MatchingInlineAsm);
  switch (MatchResult) {
  case Match_Success: {
    switch (Inst.getOpcode()) {
    default:
      Inst.setLoc(IDLoc);
      Instructions.push_back(Inst);
      break;
    case SP::SET:
      expandSET(Inst, IDLoc, Instructions);
      break;
    }

    for (const MCInst &I : Instructions) {
      Out.EmitInstruction(I, getSTI());
    }
    return false;
  }

  case Match_MissingFeature:
    return Error(IDLoc,
                 "instruction requires a CPU feature not currently enabled");

  case Match_InvalidOperand: {
    SMLoc ErrorLoc = IDLoc;
    if (ErrorInfo != ~0ULL) {
      if (ErrorInfo >= Operands.size())
        return Error(IDLoc, "too few operands for instruction");

      ErrorLoc = ((V9Operand &)*Operands[ErrorInfo]).getStartLoc();
      if (ErrorLoc == SMLoc())
        ErrorLoc = IDLoc;
    }

    return Error(ErrorLoc, "invalid operand for instruction");
  }
  case Match_MnemonicFail:
    return Error(IDLoc, "invalid instruction mnemonic");
  }
  llvm_unreachable("Implement any new match types added!");
}

bool V9AsmParser::
ParseRegister(unsigned &RegNo, SMLoc &StartLoc, SMLoc &EndLoc)
{
  const AsmToken &Tok = Parser.getTok();
  StartLoc = Tok.getLoc();
  EndLoc = Tok.getEndLoc();
  RegNo = 0;
  if (getLexer().getKind() != AsmToken::Percent)
    return false;
  Parser.Lex();
  unsigned regKind = V9Operand::rk_None;
  if (matchRegisterName(Tok, RegNo, regKind)) {
    Parser.Lex();
    return false;
  }

  return Error(StartLoc, "invalid register name");
}

static void applyMnemonicAliases(StringRef &Mnemonic, uint64_t Features,
                                 unsigned VariantID);

bool V9AsmParser::ParseInstruction(ParseInstructionInfo &Info,
                                      StringRef Name, SMLoc NameLoc,
                                      OperandVector &Operands) {

  // First operand in MCInst is instruction mnemonic.
  Operands.push_back(V9Operand::CreateToken(Name, NameLoc));

  // apply mnemonic aliases, if any, so that we can parse operands correctly.
  applyMnemonicAliases(Name, getAvailableFeatures(), 0);

  if (getLexer().isNot(AsmToken::EndOfStatement)) {
    // Read the first operand.
    if (getLexer().is(AsmToken::Comma)) {
      if (parseBranchModifiers(Operands) != MatchOperand_Success) {
        SMLoc Loc = getLexer().getLoc();
        Parser.eatToEndOfStatement();
        return Error(Loc, "unexpected token");
      }
    }
    if (parseOperand(Operands, Name) != MatchOperand_Success) {
      SMLoc Loc = getLexer().getLoc();
      Parser.eatToEndOfStatement();
      return Error(Loc, "unexpected token");
    }

    while (getLexer().is(AsmToken::Comma) || getLexer().is(AsmToken::Plus)) {
      if (getLexer().is(AsmToken::Plus)) {
      // Plus tokens are significant in software_traps (p83, sparcv8.pdf). We must capture them.
        Operands.push_back(V9Operand::CreateToken("+", Parser.getTok().getLoc()));
      }
      Parser.Lex(); // Eat the comma or plus.
      // Parse and remember the operand.
      if (parseOperand(Operands, Name) != MatchOperand_Success) {
        SMLoc Loc = getLexer().getLoc();
        Parser.eatToEndOfStatement();
        return Error(Loc, "unexpected token");
      }
    }
  }
  if (getLexer().isNot(AsmToken::EndOfStatement)) {
    SMLoc Loc = getLexer().getLoc();
    Parser.eatToEndOfStatement();
    return Error(Loc, "unexpected token");
  }
  Parser.Lex(); // Consume the EndOfStatement.
  return false;
}

bool V9AsmParser::
ParseDirective(AsmToken DirectiveID)
{
  StringRef IDVal = DirectiveID.getString();

  if (IDVal == ".byte")
    return parseDirectiveWord(1, DirectiveID.getLoc());

  if (IDVal == ".half")
    return parseDirectiveWord(2, DirectiveID.getLoc());

  if (IDVal == ".word")
    return parseDirectiveWord(4, DirectiveID.getLoc());

  if (IDVal == ".nword")
    return parseDirectiveWord(is64Bit() ? 8 : 4, DirectiveID.getLoc());

  if (is64Bit() && IDVal == ".xword")
    return parseDirectiveWord(8, DirectiveID.getLoc());

  if (IDVal == ".register") {
    // For now, ignore .register directive.
    Parser.eatToEndOfStatement();
    return false;
  }
  if (IDVal == ".proc") {
    // For compatibility, ignore this directive.
    // (It's supposed to be an "optimization" in the Sun assembler)
    Parser.eatToEndOfStatement();
    return false;
  }

  // Let the MC layer to handle other directives.
  return true;
}

bool V9AsmParser:: parseDirectiveWord(unsigned Size, SMLoc L) {
  if (getLexer().isNot(AsmToken::EndOfStatement)) {
    for (;;) {
      const MCExpr *Value;
      if (getParser().parseExpression(Value))
        return true;

      getParser().getStreamer().EmitValue(Value, Size);

      if (getLexer().is(AsmToken::EndOfStatement))
        break;

      // FIXME: Improve diagnostic.
      if (getLexer().isNot(AsmToken::Comma))
        return Error(L, "unexpected token in directive");
      Parser.Lex();
    }
  }
  Parser.Lex();
  return false;
}

V9AsmParser::OperandMatchResultTy
V9AsmParser::parseMEMOperand(OperandVector &Operands) {

  SMLoc S, E;
  unsigned BaseReg = 0;

  if (ParseRegister(BaseReg, S, E)) {
    return MatchOperand_NoMatch;
  }

  switch (getLexer().getKind()) {
  default: return MatchOperand_NoMatch;

  case AsmToken::Comma:
  case AsmToken::RBrac:
  case AsmToken::EndOfStatement:
    Operands.push_back(V9Operand::CreateMEMr(BaseReg, S, E));
    return MatchOperand_Success;

  case AsmToken:: Plus:
    Parser.Lex(); // Eat the '+'
    break;
  case AsmToken::Minus:
    break;
  }

  std::unique_ptr<V9Operand> Offset;
  OperandMatchResultTy ResTy = parseV9AsmOperand(Offset);
  if (ResTy != MatchOperand_Success || !Offset)
    return MatchOperand_NoMatch;

  Operands.push_back(
      Offset->isImm() ? V9Operand::MorphToMEMri(BaseReg, std::move(Offset))
                      : V9Operand::MorphToMEMrr(BaseReg, std::move(Offset)));

  return MatchOperand_Success;
}

V9AsmParser::OperandMatchResultTy
V9AsmParser::parseOperand(OperandVector &Operands, StringRef Mnemonic) {

  OperandMatchResultTy ResTy = MatchOperandParserImpl(Operands, Mnemonic);

  // If there wasn't a custom match, try the generic matcher below. Otherwise,
  // there was a match, but an error occurred, in which case, just return that
  // the operand parsing failed.
  if (ResTy == MatchOperand_Success || ResTy == MatchOperand_ParseFail)
    return ResTy;

  if (getLexer().is(AsmToken::LBrac)) {
    // Memory operand
    Operands.push_back(V9Operand::CreateToken("[",
                                                 Parser.getTok().getLoc()));
    Parser.Lex(); // Eat the [

    if (Mnemonic == "cas" || Mnemonic == "casx") {
      SMLoc S = Parser.getTok().getLoc();
      if (getLexer().getKind() != AsmToken::Percent)
        return MatchOperand_NoMatch;
      Parser.Lex(); // eat %

      unsigned RegNo, RegKind;
      if (!matchRegisterName(Parser.getTok(), RegNo, RegKind))
        return MatchOperand_NoMatch;

      Parser.Lex(); // Eat the identifier token.
      SMLoc E = SMLoc::getFromPointer(Parser.getTok().getLoc().getPointer()-1);
      Operands.push_back(V9Operand::CreateReg(RegNo, RegKind, S, E));
      ResTy = MatchOperand_Success;
    } else {
      ResTy = parseMEMOperand(Operands);
    }

    if (ResTy != MatchOperand_Success)
      return ResTy;

    if (!getLexer().is(AsmToken::RBrac))
      return MatchOperand_ParseFail;

    Operands.push_back(V9Operand::CreateToken("]",
                                                 Parser.getTok().getLoc()));
    Parser.Lex(); // Eat the ]

    // Parse an optional address-space identifier after the address.
    if (getLexer().is(AsmToken::Integer)) {
      std::unique_ptr<V9Operand> Op;
      ResTy = parseV9AsmOperand(Op, false);
      if (ResTy != MatchOperand_Success || !Op)
        return MatchOperand_ParseFail;
      Operands.push_back(std::move(Op));
    }
    return MatchOperand_Success;
  }

  std::unique_ptr<V9Operand> Op;

  ResTy = parseV9AsmOperand(Op, (Mnemonic == "call"));
  if (ResTy != MatchOperand_Success || !Op)
    return MatchOperand_ParseFail;

  // Push the parsed operand into the list of operands
  Operands.push_back(std::move(Op));

  return MatchOperand_Success;
}

V9AsmParser::OperandMatchResultTy
V9AsmParser::parseV9AsmOperand(std::unique_ptr<V9Operand> &Op,
                                     bool isCall) {

  SMLoc S = Parser.getTok().getLoc();
  SMLoc E = SMLoc::getFromPointer(Parser.getTok().getLoc().getPointer() - 1);
  const MCExpr *EVal;

  Op = nullptr;
  switch (getLexer().getKind()) {
  default:  break;

  case AsmToken::Percent:
    Parser.Lex(); // Eat the '%'.
    unsigned RegNo;
    unsigned RegKind;
    if (matchRegisterName(Parser.getTok(), RegNo, RegKind)) {
      StringRef name = Parser.getTok().getString();
      Parser.Lex(); // Eat the identifier token.
      E = SMLoc::getFromPointer(Parser.getTok().getLoc().getPointer() - 1);
      switch (RegNo) {
      default:
        Op = V9Operand::CreateReg(RegNo, RegKind, S, E);
        break;
      case V9::PSR:
        Op = V9Operand::CreateToken("%psr", S);
        break;
      case V9::FSR:
        Op = V9Operand::CreateToken("%fsr", S);
        break;
      case V9::FQ:
        Op = V9Operand::CreateToken("%fq", S);
        break;
      case V9::CPSR:
        Op = V9Operand::CreateToken("%csr", S);
        break;
      case V9::CPQ:
        Op = V9Operand::CreateToken("%cq", S);
        break;
      case V9::WIM:
        Op = V9Operand::CreateToken("%wim", S);
        break;
      case V9::TBR:
        Op = V9Operand::CreateToken("%tbr", S);
        break;
      case V9::ICC:
        if (name == "xcc")
          Op = V9Operand::CreateToken("%xcc", S);
        else
          Op = V9Operand::CreateToken("%icc", S);
        break;
      }
      break;
    }
    if (matchV9AsmModifiers(EVal, E)) {
      E = SMLoc::getFromPointer(Parser.getTok().getLoc().getPointer() - 1);
      Op = V9Operand::CreateImm(EVal, S, E);
    }
    break;

  case AsmToken::Minus:
  case AsmToken::Integer:
  case AsmToken::LParen:
  case AsmToken::Dot:
    if (!getParser().parseExpression(EVal, E))
      Op = V9Operand::CreateImm(EVal, S, E);
    break;

  case AsmToken::Identifier: {
    StringRef Identifier;
    if (!getParser().parseIdentifier(Identifier)) {
      E = SMLoc::getFromPointer(Parser.getTok().getLoc().getPointer() - 1);
      MCSymbol *Sym = getContext().getOrCreateSymbol(Identifier);

      const MCExpr *Res = MCSymbolRefExpr::create(Sym, MCSymbolRefExpr::VK_None,
                                                  getContext());
      if (isCall &&
          getContext().getObjectFileInfo()->getRelocM() == Reloc::PIC_)
        Res = V9MCExpr::create(V9MCExpr::VK_V9_WPLT30, Res,
                                  getContext());
      Op = V9Operand::CreateImm(Res, S, E);
    }
    break;
  }
  }
  return (Op) ? MatchOperand_Success : MatchOperand_ParseFail;
}

V9AsmParser::OperandMatchResultTy
V9AsmParser::parseBranchModifiers(OperandVector &Operands) {

  // parse (,a|,pn|,pt)+

  while (getLexer().is(AsmToken::Comma)) {

    Parser.Lex(); // Eat the comma

    if (!getLexer().is(AsmToken::Identifier))
      return MatchOperand_ParseFail;
    StringRef modName = Parser.getTok().getString();
    if (modName == "a" || modName == "pn" || modName == "pt") {
      Operands.push_back(V9Operand::CreateToken(modName,
                                                   Parser.getTok().getLoc()));
      Parser.Lex(); // eat the identifier.
    }
  }
  return MatchOperand_Success;
}

bool V9AsmParser::matchRegisterName(const AsmToken &Tok,
                                       unsigned &RegNo,
                                       unsigned &RegKind)
{
  int64_t intVal = 0;
  RegNo = 0;
  RegKind = V9Operand::rk_None;
  if (Tok.is(AsmToken::Identifier)) {
    StringRef name = Tok.getString();

    // %fp
    if (name.equals("fp")) {
      RegNo = V9::I6;
      RegKind = V9Operand::rk_IntReg;
      return true;
    }
    // %sp
    if (name.equals("sp")) {
      RegNo = V9::O6;
      RegKind = V9Operand::rk_IntReg;
      return true;
    }

    if (name.equals("y")) {
      RegNo = V9::Y;
      RegKind = V9Operand::rk_Special;
      return true;
    }

    if (name.substr(0, 3).equals_lower("asr")
        && !name.substr(3).getAsInteger(10, intVal)
        && intVal > 0 && intVal < 32) {
      RegNo = ASRRegs[intVal];
      RegKind = V9Operand::rk_Special;
      return true;
    }

    // %fprs is an alias of %asr6.
    if (name.equals("fprs")) {
      RegNo = ASRRegs[6];
      RegKind = V9Operand::rk_Special;
      return true;
    }

    if (name.equals("icc")) {
      RegNo = V9::ICC;
      RegKind = V9Operand::rk_Special;
      return true;
    }

    if (name.equals("psr")) {
      RegNo = V9::PSR;
      RegKind = V9Operand::rk_Special;
      return true;
    }

    if (name.equals("fsr")) {
      RegNo = V9::FSR;
      RegKind = V9Operand::rk_Special;
      return true;
    }

    if (name.equals("fq")) {
      RegNo = V9::FQ;
      RegKind = V9Operand::rk_Special;
      return true;
    }

    if (name.equals("csr")) {
      RegNo = V9::CPSR;
      RegKind = V9Operand::rk_Special;
      return true;
    }

    if (name.equals("cq")) {
      RegNo = V9::CPQ;
      RegKind = V9Operand::rk_Special;
      return true;
    }
    
    if (name.equals("wim")) {
      RegNo = V9::WIM;
      RegKind = V9Operand::rk_Special;
      return true;
    }

    if (name.equals("tbr")) {
      RegNo = V9::TBR;
      RegKind = V9Operand::rk_Special;
      return true;
    }

    if (name.equals("xcc")) {
      // FIXME:: check 64bit.
      RegNo = V9::ICC;
      RegKind = V9Operand::rk_Special;
      return true;
    }

    // %fcc0 - %fcc3
    if (name.substr(0, 3).equals_lower("fcc")
        && !name.substr(3).getAsInteger(10, intVal)
        && intVal < 4) {
      // FIXME: check 64bit and  handle %fcc1 - %fcc3
      RegNo = V9::FCC0 + intVal;
      RegKind = V9Operand::rk_Special;
      return true;
    }

    // %g0 - %g7
    if (name.substr(0, 1).equals_lower("g")
        && !name.substr(1).getAsInteger(10, intVal)
        && intVal < 8) {
      RegNo = IntRegs[intVal];
      RegKind = V9Operand::rk_IntReg;
      return true;
    }
    // %o0 - %o7
    if (name.substr(0, 1).equals_lower("o")
        && !name.substr(1).getAsInteger(10, intVal)
        && intVal < 8) {
      RegNo = IntRegs[8 + intVal];
      RegKind = V9Operand::rk_IntReg;
      return true;
    }
    if (name.substr(0, 1).equals_lower("l")
        && !name.substr(1).getAsInteger(10, intVal)
        && intVal < 8) {
      RegNo = IntRegs[16 + intVal];
      RegKind = V9Operand::rk_IntReg;
      return true;
    }
    if (name.substr(0, 1).equals_lower("i")
        && !name.substr(1).getAsInteger(10, intVal)
        && intVal < 8) {
      RegNo = IntRegs[24 + intVal];
      RegKind = V9Operand::rk_IntReg;
      return true;
    }
    // %f0 - %f31
    if (name.substr(0, 1).equals_lower("f")
        && !name.substr(1, 2).getAsInteger(10, intVal) && intVal < 32) {
      RegNo = FloatRegs[intVal];
      RegKind = V9Operand::rk_FloatReg;
      return true;
    }
    // %f32 - %f62
    if (name.substr(0, 1).equals_lower("f")
        && !name.substr(1, 2).getAsInteger(10, intVal)
        && intVal >= 32 && intVal <= 62 && (intVal % 2 == 0)) {
      // FIXME: Check V9
      RegNo = DoubleRegs[intVal/2];
      RegKind = V9Operand::rk_DoubleReg;
      return true;
    }

    // %r0 - %r31
    if (name.substr(0, 1).equals_lower("r")
        && !name.substr(1, 2).getAsInteger(10, intVal) && intVal < 31) {
      RegNo = IntRegs[intVal];
      RegKind = V9Operand::rk_IntReg;
      return true;
    }

    // %c0 - %c31
    if (name.substr(0, 1).equals_lower("c")
        && !name.substr(1).getAsInteger(10, intVal)
        && intVal < 32) {
      RegNo = CoprocRegs[intVal];
      RegKind = V9Operand::rk_CoprocReg;
      return true;
    }
    
    if (name.equals("tpc")) {
      RegNo = V9::TPC;
      RegKind = V9Operand::rk_Special;
      return true;
    }
    if (name.equals("tnpc")) {
      RegNo = V9::TNPC;
      RegKind = V9Operand::rk_Special;
      return true;
    }
    if (name.equals("tstate")) {
      RegNo = V9::TSTATE;
      RegKind = V9Operand::rk_Special;
      return true;
    }
    if (name.equals("tt")) {
      RegNo = V9::TT;
      RegKind = V9Operand::rk_Special;
      return true;
    }
    if (name.equals("tick")) {
      RegNo = V9::TICK;
      RegKind = V9Operand::rk_Special;
      return true;
    }
    if (name.equals("tba")) {
      RegNo = V9::TBA;
      RegKind = V9Operand::rk_Special;
      return true;
    }
    if (name.equals("pstate")) {
      RegNo = V9::PSTATE;
      RegKind = V9Operand::rk_Special;
      return true;
    }
    if (name.equals("tl")) {
      RegNo = V9::TL;
      RegKind = V9Operand::rk_Special;
      return true;
    }
    if (name.equals("pil")) {
      RegNo = V9::PIL;
      RegKind = V9Operand::rk_Special;
      return true;
    }
    if (name.equals("cwp")) {
      RegNo = V9::CWP;
      RegKind = V9Operand::rk_Special;
      return true;
    }
    if (name.equals("cansave")) {
      RegNo = V9::CANSAVE;
      RegKind = V9Operand::rk_Special;
      return true;
    }
    if (name.equals("canrestore")) {
      RegNo = V9::CANRESTORE;
      RegKind = V9Operand::rk_Special;
      return true;
    }
    if (name.equals("cleanwin")) {
      RegNo = V9::CLEANWIN;
      RegKind = V9Operand::rk_Special;
      return true;
    }
    if (name.equals("otherwin")) {
      RegNo = V9::OTHERWIN;
      RegKind = V9Operand::rk_Special;
      return true;
    }
    if (name.equals("wstate")) {
      RegNo = V9::WSTATE;
      RegKind = V9Operand::rk_Special;
      return true;
    }
  }
  return false;
}

// Determine if an expression contains a reference to the symbol
// "_GLOBAL_OFFSET_TABLE_".
static bool hasGOTReference(const MCExpr *Expr) {
  switch (Expr->getKind()) {
  case MCExpr::Target:
    if (const V9MCExpr *SE = dyn_cast<V9MCExpr>(Expr))
      return hasGOTReference(SE->getSubExpr());
    break;

  case MCExpr::Constant:
    break;

  case MCExpr::Binary: {
    const MCBinaryExpr *BE = cast<MCBinaryExpr>(Expr);
    return hasGOTReference(BE->getLHS()) || hasGOTReference(BE->getRHS());
  }

  case MCExpr::SymbolRef: {
    const MCSymbolRefExpr &SymRef = *cast<MCSymbolRefExpr>(Expr);
    return (SymRef.getSymbol().getName() == "_GLOBAL_OFFSET_TABLE_");
  }

  case MCExpr::Unary:
    return hasGOTReference(cast<MCUnaryExpr>(Expr)->getSubExpr());
  }
  return false;
}

const V9MCExpr *
V9AsmParser::adjustPICRelocation(V9MCExpr::VariantKind VK,
                                    const MCExpr *subExpr)
{
  // When in PIC mode, "%lo(...)" and "%hi(...)" behave differently.
  // If the expression refers contains _GLOBAL_OFFSETE_TABLE, it is
  // actually a %pc10 or %pc22 relocation. Otherwise, they are interpreted
  // as %got10 or %got22 relocation.

  if (getContext().getObjectFileInfo()->getRelocM() == Reloc::PIC_) {
    switch(VK) {
    default: break;
    case V9MCExpr::VK_V9_LO:
      VK = (hasGOTReference(subExpr) ? V9MCExpr::VK_V9_PC10
                                     : V9MCExpr::VK_V9_GOT10);
      break;
    case V9MCExpr::VK_V9_HI:
      VK = (hasGOTReference(subExpr) ? V9MCExpr::VK_V9_PC22
                                     : V9MCExpr::VK_V9_GOT22);
      break;
    }
  }

  return V9MCExpr::create(VK, subExpr, getContext());
}

bool V9AsmParser::matchV9AsmModifiers(const MCExpr *&EVal,
                                            SMLoc &EndLoc)
{
  AsmToken Tok = Parser.getTok();
  if (!Tok.is(AsmToken::Identifier))
    return false;

  StringRef name = Tok.getString();

  V9MCExpr::VariantKind VK = V9MCExpr::parseVariantKind(name);

  if (VK == V9MCExpr::VK_V9_None)
    return false;

  Parser.Lex(); // Eat the identifier.
  if (Parser.getTok().getKind() != AsmToken::LParen)
    return false;

  Parser.Lex(); // Eat the LParen token.
  const MCExpr *subExpr;
  if (Parser.parseParenExpression(subExpr, EndLoc))
    return false;

  EVal = adjustPICRelocation(VK, subExpr);
  return true;
}

extern "C" void LLVMInitializeV9AsmParser() {
  RegisterMCAsmParser<V9AsmParser> A(TheV9Target);
  RegisterMCAsmParser<V9AsmParser> B(TheV9V9Target);
  RegisterMCAsmParser<V9AsmParser> C(TheV9elTarget);
}

#define GET_REGISTER_MATCHER
#define GET_MATCHER_IMPLEMENTATION
#include "V9GenAsmMatcher.inc"

unsigned V9AsmParser::validateTargetOperandClass(MCParsedAsmOperand &GOp,
                                                    unsigned Kind) {
  V9Operand &Op = (V9Operand &)GOp;
  if (Op.isFloatOrDoubleReg()) {
    switch (Kind) {
    default: break;
    case MCK_DFPRegs:
      if (!Op.isFloatReg() || V9Operand::MorphToDoubleReg(Op))
        return MCTargetAsmParser::Match_Success;
      break;
    case MCK_QFPRegs:
      if (V9Operand::MorphToQuadReg(Op))
        return MCTargetAsmParser::Match_Success;
      break;
    }
  }
  if (Op.isIntReg() && Kind == MCK_IntPair) {
    if (V9Operand::MorphToIntPairReg(Op))
      return MCTargetAsmParser::Match_Success;
  }
  if (Op.isCoprocReg() && Kind == MCK_CoprocPair) {
     if (V9Operand::MorphToCoprocPairReg(Op))
       return MCTargetAsmParser::Match_Success;
   }
  return Match_InvalidOperand;
}
