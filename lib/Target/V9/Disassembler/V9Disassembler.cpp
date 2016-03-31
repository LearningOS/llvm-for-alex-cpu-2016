//===- V9Disassembler.cpp - Disassembler for V9 -----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file is part of the V9 Disassembler.
//
//===----------------------------------------------------------------------===//

#include "V9.h"
#include "V9RegisterInfo.h"
#include "V9Subtarget.h"
#include "llvm/MC/MCDisassembler/MCDisassembler.h"
#include "llvm/MC/MCFixedLenDisassembler.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

#define DEBUG_TYPE "sparc-disassembler"

typedef MCDisassembler::DecodeStatus DecodeStatus;

namespace {

/// A disassembler class for V9.
class V9Disassembler : public MCDisassembler {
public:
  V9Disassembler(const MCSubtargetInfo &STI, MCContext &Ctx)
      : MCDisassembler(STI, Ctx) {}
  virtual ~V9Disassembler() {}

  DecodeStatus getInstruction(MCInst &Instr, uint64_t &Size,
                              ArrayRef<uint8_t> Bytes, uint64_t Address,
                              raw_ostream &VStream,
                              raw_ostream &CStream) const override;
};
}

namespace llvm {
extern Target TheV9Target, TheV9V9Target, TheV9elTarget;
}

static MCDisassembler *createV9Disassembler(const Target &T,
                                               const MCSubtargetInfo &STI,
                                               MCContext &Ctx) {
  return new V9Disassembler(STI, Ctx);
}


extern "C" void LLVMInitializeV9Disassembler() {
  // Register the disassembler.
  TargetRegistry::RegisterMCDisassembler(TheV9Target,
                                         createV9Disassembler);
  TargetRegistry::RegisterMCDisassembler(TheV9V9Target,
                                         createV9Disassembler);
  TargetRegistry::RegisterMCDisassembler(TheV9elTarget,
                                         createV9Disassembler);
}

static const unsigned IntRegDecoderTable[] = {
  SP::G0,  SP::G1,  SP::G2,  SP::G3,
  SP::G4,  SP::G5,  SP::G6,  SP::G7,
  SP::O0,  SP::O1,  SP::O2,  SP::O3,
  SP::O4,  SP::O5,  SP::O6,  SP::O7,
  SP::A0,  SP::A1,  SP::A2,  SP::A3,
  SP::A4,  SP::A5,  SP::A6,  SP::A7 };

static const unsigned FPRegDecoderTable[] = {
  SP::F0,   SP::F1,   SP::F2,   SP::F3,
  SP::F4,   SP::F5,   SP::F6,   SP::F7,
  SP::F8,   SP::F9,   SP::F10,  SP::F11,
  SP::F12,  SP::F13,  SP::F14,  SP::F15,
  SP::F16,  SP::F17,  SP::F18,  SP::F19,
  SP::F20,  SP::F21,  SP::F22,  SP::F23,
  SP::F24,  SP::F25,  SP::F26,  SP::F27,
  SP::F28,  SP::F29,  SP::F30,  SP::F31 };

static const unsigned DFPRegDecoderTable[] = {
  SP::D0,   SP::D16,  SP::D1,   SP::D17,
  SP::D2,   SP::D18,  SP::D3,   SP::D19,
  SP::D4,   SP::D20,  SP::D5,   SP::D21,
  SP::D6,   SP::D22,  SP::D7,   SP::D23,
  SP::D8,   SP::D24,  SP::D9,   SP::D25,
  SP::D10,  SP::D26,  SP::D11,  SP::D27,
  SP::D12,  SP::D28,  SP::D13,  SP::D29,
  SP::D14,  SP::D30,  SP::D15,  SP::D31 };

static const unsigned QFPRegDecoderTable[] = {
  SP::Q0,  SP::Q8,   ~0U,  ~0U,
  SP::Q1,  SP::Q9,   ~0U,  ~0U,
  SP::Q2,  SP::Q10,  ~0U,  ~0U,
  SP::Q3,  SP::Q11,  ~0U,  ~0U,
  SP::Q4,  SP::Q12,  ~0U,  ~0U,
  SP::Q5,  SP::Q13,  ~0U,  ~0U,
  SP::Q6,  SP::Q14,  ~0U,  ~0U,
  SP::Q7,  SP::Q15,  ~0U,  ~0U } ;

static const unsigned FCCRegDecoderTable[] = {
  SP::FCC0, SP::FCC1, SP::FCC2, SP::FCC3 };

static const unsigned ASRRegDecoderTable[] = {
  SP::Y,     SP::ASR1,  SP::ASR2,  SP::ASR3,
  SP::ASR4,  SP::ASR5,  SP::ASR6,  SP::ASR7,
  SP::ASR8,  SP::ASR9,  SP::ASR10, SP::ASR11,
  SP::ASR12, SP::ASR13, SP::ASR14, SP::ASR15,
  SP::ASR16, SP::ASR17, SP::ASR18, SP::ASR19,
  SP::ASR20, SP::ASR21, SP::ASR22, SP::ASR23,
  SP::ASR24, SP::ASR25, SP::ASR26, SP::ASR27,
  SP::ASR28, SP::ASR29, SP::ASR30, SP::ASR31};

static const unsigned PRRegDecoderTable[] = {
  SP::TPC, SP::TNPC, SP::TSTATE, SP::TT, SP::TICK, SP::TBA, SP::PSTATE,
  SP::TL, SP::PIL, SP::CWP, SP::CANSAVE, SP::CANRESTORE, SP::CLEANWIN,
  SP::OTHERWIN, SP::WSTATE
};

static const unsigned CPRegDecoderTable[] = {
  SP::C0,  SP::C1,  SP::C2,  SP::C3,
  SP::C4,  SP::C5,  SP::C6,  SP::C7,
  SP::C8,  SP::C9,  SP::C10, SP::C11,
  SP::C12, SP::C13, SP::C14, SP::C15,
  SP::C16, SP::C17, SP::C18, SP::C19,
  SP::C20, SP::C21, SP::C22, SP::C23,
  SP::C24, SP::C25, SP::C26, SP::C27,
  SP::C28, SP::C29, SP::C30, SP::C31
};


static const uint16_t CPPairDecoderTable[] = {
  SP::C0_C1,   SP::C2_C3,   SP::C4_C5,   SP::C6_C7,
  SP::C8_C9,   SP::C10_C11, SP::C12_C13, SP::C14_C15,
  SP::C16_C17, SP::C18_C19, SP::C20_C21, SP::C22_C23,
  SP::C24_C25, SP::C26_C27, SP::C28_C29, SP::C30_C31
};

static DecodeStatus DecodeIntRegsRegisterClass(MCInst &Inst,
                                               unsigned RegNo,
                                               uint64_t Address,
                                               const void *Decoder) {
  if (RegNo > 31)
    return MCDisassembler::Fail;
  unsigned Reg = IntRegDecoderTable[RegNo];
  Inst.addOperand(MCOperand::createReg(Reg));
  return MCDisassembler::Success;
}

static DecodeStatus DecodeI64RegsRegisterClass(MCInst &Inst,
                                               unsigned RegNo,
                                               uint64_t Address,
                                               const void *Decoder) {
  if (RegNo > 31)
    return MCDisassembler::Fail;
  unsigned Reg = IntRegDecoderTable[RegNo];
  Inst.addOperand(MCOperand::createReg(Reg));
  return MCDisassembler::Success;
}


static DecodeStatus DecodeFPRegsRegisterClass(MCInst &Inst,
                                              unsigned RegNo,
                                              uint64_t Address,
                                              const void *Decoder) {
  if (RegNo > 31)
    return MCDisassembler::Fail;
  unsigned Reg = FPRegDecoderTable[RegNo];
  Inst.addOperand(MCOperand::createReg(Reg));
  return MCDisassembler::Success;
}


static DecodeStatus DecodeDFPRegsRegisterClass(MCInst &Inst,
                                               unsigned RegNo,
                                               uint64_t Address,
                                               const void *Decoder) {
  if (RegNo > 31)
    return MCDisassembler::Fail;
  unsigned Reg = DFPRegDecoderTable[RegNo];
  Inst.addOperand(MCOperand::createReg(Reg));
  return MCDisassembler::Success;
}


static DecodeStatus DecodeQFPRegsRegisterClass(MCInst &Inst,
                                               unsigned RegNo,
                                               uint64_t Address,
                                               const void *Decoder) {
  if (RegNo > 31)
    return MCDisassembler::Fail;

  unsigned Reg = QFPRegDecoderTable[RegNo];
  if (Reg == ~0U)
    return MCDisassembler::Fail;
  Inst.addOperand(MCOperand::createReg(Reg));
  return MCDisassembler::Success;
}

static DecodeStatus DecodeCPRegsRegisterClass(MCInst &Inst,
                                               unsigned RegNo,
                                               uint64_t Address,
                                               const void *Decoder) {
  if (RegNo > 31)
    return MCDisassembler::Fail;
  unsigned Reg = CPRegDecoderTable[RegNo];
  Inst.addOperand(MCOperand::createReg(Reg));
  return MCDisassembler::Success;
}

static DecodeStatus DecodeFCCRegsRegisterClass(MCInst &Inst, unsigned RegNo,
                                               uint64_t Address,
                                               const void *Decoder) {
  if (RegNo > 3)
    return MCDisassembler::Fail;
  Inst.addOperand(MCOperand::createReg(FCCRegDecoderTable[RegNo]));
  return MCDisassembler::Success;
}

static DecodeStatus DecodeASRRegsRegisterClass(MCInst &Inst, unsigned RegNo,
                                               uint64_t Address,
                                               const void *Decoder) {
  if (RegNo > 31)
    return MCDisassembler::Fail;
  Inst.addOperand(MCOperand::createReg(ASRRegDecoderTable[RegNo]));
  return MCDisassembler::Success;
}

static DecodeStatus DecodePRRegsRegisterClass(MCInst &Inst, unsigned RegNo,
                                               uint64_t Address,
                                               const void *Decoder) {
  if (RegNo >= array_lengthof(PRRegDecoderTable))
    return MCDisassembler::Fail;
  Inst.addOperand(MCOperand::createReg(PRRegDecoderTable[RegNo]));
  return MCDisassembler::Success;
}

static DecodeStatus DecodeIntPairRegisterClass(MCInst &Inst, unsigned RegNo,
                                   uint64_t Address, const void *Decoder) {
  DecodeStatus S = MCDisassembler::Success;

  if (RegNo > 31)
    return MCDisassembler::Fail;

  if ((RegNo & 1))
    S = MCDisassembler::SoftFail;

  return S;
}

static DecodeStatus DecodeCPPairRegisterClass(MCInst &Inst, unsigned RegNo,
                                   uint64_t Address, const void *Decoder) {
  if (RegNo > 31)
    return MCDisassembler::Fail;

  unsigned RegisterPair = CPPairDecoderTable[RegNo/2];
  Inst.addOperand(MCOperand::createReg(RegisterPair));
  return MCDisassembler::Success;
}

static DecodeStatus DecodeLoadInt(MCInst &Inst, unsigned insn, uint64_t Address,
                                  const void *Decoder);
static DecodeStatus DecodeLoadIntPair(MCInst &Inst, unsigned insn, uint64_t Address,
                                  const void *Decoder);
static DecodeStatus DecodeLoadFP(MCInst &Inst, unsigned insn, uint64_t Address,
                                 const void *Decoder);
static DecodeStatus DecodeLoadDFP(MCInst &Inst, unsigned insn, uint64_t Address,
                                  const void *Decoder);
static DecodeStatus DecodeLoadQFP(MCInst &Inst, unsigned insn, uint64_t Address,
                                  const void *Decoder);
static DecodeStatus DecodeLoadCP(MCInst &Inst, unsigned insn, uint64_t Address,
                                  const void *Decoder);
static DecodeStatus DecodeLoadCPPair(MCInst &Inst, unsigned insn, uint64_t Address,
                                  const void *Decoder);
static DecodeStatus DecodeStoreInt(MCInst &Inst, unsigned insn,
                                   uint64_t Address, const void *Decoder);
static DecodeStatus DecodeStoreIntPair(MCInst &Inst, unsigned insn,
                                   uint64_t Address, const void *Decoder);
static DecodeStatus DecodeStoreFP(MCInst &Inst, unsigned insn,
                                  uint64_t Address, const void *Decoder);
static DecodeStatus DecodeStoreDFP(MCInst &Inst, unsigned insn,
                                   uint64_t Address, const void *Decoder);
static DecodeStatus DecodeStoreQFP(MCInst &Inst, unsigned insn,
                                   uint64_t Address, const void *Decoder);
static DecodeStatus DecodeStoreCP(MCInst &Inst, unsigned insn,
                                   uint64_t Address, const void *Decoder);
static DecodeStatus DecodeStoreCPPair(MCInst &Inst, unsigned insn,
                                   uint64_t Address, const void *Decoder);
static DecodeStatus DecodeCall(MCInst &Inst, unsigned insn,
                               uint64_t Address, const void *Decoder);
static DecodeStatus DecodeSIMM13(MCInst &Inst, unsigned insn,
                                 uint64_t Address, const void *Decoder);
static DecodeStatus DecodeJMPL(MCInst &Inst, unsigned insn, uint64_t Address,
                               const void *Decoder);
static DecodeStatus DecodeReturn(MCInst &MI, unsigned insn, uint64_t Address,
                                 const void *Decoder);
static DecodeStatus DecodeSWAP(MCInst &Inst, unsigned insn, uint64_t Address,
                               const void *Decoder);
static DecodeStatus DecodeTRAP(MCInst &Inst, unsigned insn, uint64_t Address,
                               const void *Decoder);

#include "V9GenDisassemblerTables.inc"

/// Read four bytes from the ArrayRef and return 32 bit word.
static DecodeStatus readInstruction32(ArrayRef<uint8_t> Bytes, uint64_t Address,
                                      uint64_t &Size, uint32_t &Insn,
                                      bool IsLittleEndian) {
  // We want to read exactly 4 Bytes of data.
  if (Bytes.size() < 4) {
    Size = 0;
    return MCDisassembler::Fail;
  }

  Insn = IsLittleEndian
             ? (Bytes[0] << 0) | (Bytes[1] << 8) | (Bytes[2] << 16) |
                   (Bytes[3] << 24)
             : (Bytes[3] << 0) | (Bytes[2] << 8) | (Bytes[1] << 16) |
                   (Bytes[0] << 24);

  return MCDisassembler::Success;
}

DecodeStatus V9Disassembler::getInstruction(MCInst &Instr, uint64_t &Size,
                                               ArrayRef<uint8_t> Bytes,
                                               uint64_t Address,
                                               raw_ostream &VStream,
                                               raw_ostream &CStream) const {
  uint32_t Insn;
  bool isLittleEndian = getContext().getAsmInfo()->isLittleEndian();
  DecodeStatus Result =
      readInstruction32(Bytes, Address, Size, Insn, isLittleEndian);
  if (Result == MCDisassembler::Fail)
    return MCDisassembler::Fail;

  // Calling the auto-generated decoder function.
  
  if (STI.getFeatureBits()[V9::FeatureV9])
  {
    Result = decodeInstruction(DecoderTableV9V932, Instr, Insn, Address, this, STI);
  }
  else
  {
    Result = decodeInstruction(DecoderTableV9V832, Instr, Insn, Address, this, STI);      
  }
  if (Result != MCDisassembler::Fail)
    return Result;
  
  Result =
      decodeInstruction(DecoderTableV932, Instr, Insn, Address, this, STI);

  if (Result != MCDisassembler::Fail) {
    Size = 4;
    return Result;
  }

  return MCDisassembler::Fail;
}


typedef DecodeStatus (*DecodeFunc)(MCInst &MI, unsigned insn, uint64_t Address,
                                   const void *Decoder);

static DecodeStatus DecodeMem(MCInst &MI, unsigned insn, uint64_t Address,
                              const void *Decoder,
                              bool isLoad, DecodeFunc DecodeRD) {
  unsigned rd = fieldFromInstruction(insn, 25, 5);
  unsigned rs1 = fieldFromInstruction(insn, 14, 5);
  bool isImm = fieldFromInstruction(insn, 13, 1);
  bool hasAsi = fieldFromInstruction(insn, 23, 1); // (in op3 field)
  unsigned asi = fieldFromInstruction(insn, 5, 8);
  unsigned rs2 = 0;
  unsigned simm13 = 0;
  if (isImm)
    simm13 = SignExtend32<13>(fieldFromInstruction(insn, 0, 13));
  else
    rs2 = fieldFromInstruction(insn, 0, 5);

  DecodeStatus status;
  if (isLoad) {
    status = DecodeRD(MI, rd, Address, Decoder);
    if (status != MCDisassembler::Success)
      return status;
  }

  // Decode rs1.
  status = DecodeIntRegsRegisterClass(MI, rs1, Address, Decoder);
  if (status != MCDisassembler::Success)
    return status;

  // Decode imm|rs2.
  if (isImm)
    MI.addOperand(MCOperand::createImm(simm13));
  else {
    status = DecodeIntRegsRegisterClass(MI, rs2, Address, Decoder);
    if (status != MCDisassembler::Success)
      return status;
  }

  if (hasAsi)
    MI.addOperand(MCOperand::createImm(asi));

  if (!isLoad) {
    status = DecodeRD(MI, rd, Address, Decoder);
    if (status != MCDisassembler::Success)
      return status;
  }
  return MCDisassembler::Success;
}

static DecodeStatus DecodeLoadInt(MCInst &Inst, unsigned insn, uint64_t Address,
                                  const void *Decoder) {
  return DecodeMem(Inst, insn, Address, Decoder, true,
                   DecodeIntRegsRegisterClass);
}

static DecodeStatus DecodeLoadIntPair(MCInst &Inst, unsigned insn, uint64_t Address,
                                  const void *Decoder) {
  return DecodeMem(Inst, insn, Address, Decoder, true,
                   DecodeIntPairRegisterClass);
}

static DecodeStatus DecodeLoadFP(MCInst &Inst, unsigned insn, uint64_t Address,
                                 const void *Decoder) {
  return DecodeMem(Inst, insn, Address, Decoder, true,
                   DecodeFPRegsRegisterClass);
}

static DecodeStatus DecodeLoadDFP(MCInst &Inst, unsigned insn, uint64_t Address,
                                  const void *Decoder) {
  return DecodeMem(Inst, insn, Address, Decoder, true,
                   DecodeDFPRegsRegisterClass);
}

static DecodeStatus DecodeLoadQFP(MCInst &Inst, unsigned insn, uint64_t Address,
                                  const void *Decoder) {
  return DecodeMem(Inst, insn, Address, Decoder, true,
                   DecodeQFPRegsRegisterClass);
}

static DecodeStatus DecodeLoadCP(MCInst &Inst, unsigned insn, uint64_t Address,
                                  const void *Decoder) {
  return DecodeMem(Inst, insn, Address, Decoder, true,
                   DecodeCPRegsRegisterClass);
}

static DecodeStatus DecodeLoadCPPair(MCInst &Inst, unsigned insn, uint64_t Address,
                                  const void *Decoder) {
  return DecodeMem(Inst, insn, Address, Decoder, true,
                   DecodeCPPairRegisterClass);
}

static DecodeStatus DecodeStoreInt(MCInst &Inst, unsigned insn,
                                   uint64_t Address, const void *Decoder) {
  return DecodeMem(Inst, insn, Address, Decoder, false,
                   DecodeIntRegsRegisterClass);
}

static DecodeStatus DecodeStoreIntPair(MCInst &Inst, unsigned insn,
                                   uint64_t Address, const void *Decoder) {
  return DecodeMem(Inst, insn, Address, Decoder, false,
                   DecodeIntPairRegisterClass);
}

static DecodeStatus DecodeStoreFP(MCInst &Inst, unsigned insn, uint64_t Address,
                                  const void *Decoder) {
  return DecodeMem(Inst, insn, Address, Decoder, false,
                   DecodeFPRegsRegisterClass);
}

static DecodeStatus DecodeStoreDFP(MCInst &Inst, unsigned insn,
                                   uint64_t Address, const void *Decoder) {
  return DecodeMem(Inst, insn, Address, Decoder, false,
                   DecodeDFPRegsRegisterClass);
}

static DecodeStatus DecodeStoreQFP(MCInst &Inst, unsigned insn,
                                   uint64_t Address, const void *Decoder) {
  return DecodeMem(Inst, insn, Address, Decoder, false,
                   DecodeQFPRegsRegisterClass);
}

static DecodeStatus DecodeStoreCP(MCInst &Inst, unsigned insn,
                                   uint64_t Address, const void *Decoder) {
  return DecodeMem(Inst, insn, Address, Decoder, false,
                   DecodeCPRegsRegisterClass);
}

static DecodeStatus DecodeStoreCPPair(MCInst &Inst, unsigned insn,
                                   uint64_t Address, const void *Decoder) {
  return DecodeMem(Inst, insn, Address, Decoder, false,
                   DecodeCPPairRegisterClass);
}

static bool tryAddingSymbolicOperand(int64_t Value,  bool isBranch,
                                     uint64_t Address, uint64_t Offset,
                                     uint64_t Width, MCInst &MI,
                                     const void *Decoder) {
  const MCDisassembler *Dis = static_cast<const MCDisassembler*>(Decoder);
  return Dis->tryAddingSymbolicOperand(MI, Value, Address, isBranch,
                                       Offset, Width);
}

static DecodeStatus DecodeCall(MCInst &MI, unsigned insn,
                               uint64_t Address, const void *Decoder) {
  unsigned tgt = fieldFromInstruction(insn, 0, 30);
  tgt <<= 2;
  if (!tryAddingSymbolicOperand(tgt+Address, false, Address,
                                0, 30, MI, Decoder))
    MI.addOperand(MCOperand::createImm(tgt));
  return MCDisassembler::Success;
}

static DecodeStatus DecodeSIMM13(MCInst &MI, unsigned insn,
                                 uint64_t Address, const void *Decoder) {
  unsigned tgt = SignExtend32<13>(fieldFromInstruction(insn, 0, 13));
  MI.addOperand(MCOperand::createImm(tgt));
  return MCDisassembler::Success;
}

static DecodeStatus DecodeJMPL(MCInst &MI, unsigned insn, uint64_t Address,
                               const void *Decoder) {

  unsigned rd = fieldFromInstruction(insn, 25, 5);
  unsigned rs1 = fieldFromInstruction(insn, 14, 5);
  unsigned isImm = fieldFromInstruction(insn, 13, 1);
  unsigned rs2 = 0;
  unsigned simm13 = 0;
  if (isImm)
    simm13 = SignExtend32<13>(fieldFromInstruction(insn, 0, 13));
  else
    rs2 = fieldFromInstruction(insn, 0, 5);

  // Decode RD.
  DecodeStatus status = DecodeIntRegsRegisterClass(MI, rd, Address, Decoder);
  if (status != MCDisassembler::Success)
    return status;

  // Decode RS1.
  status = DecodeIntRegsRegisterClass(MI, rs1, Address, Decoder);
  if (status != MCDisassembler::Success)
    return status;

  // Decode RS1 | SIMM13.
  if (isImm)
    MI.addOperand(MCOperand::createImm(simm13));
  else {
    status = DecodeIntRegsRegisterClass(MI, rs2, Address, Decoder);
    if (status != MCDisassembler::Success)
      return status;
  }
  return MCDisassembler::Success;
}

static DecodeStatus DecodeReturn(MCInst &MI, unsigned insn, uint64_t Address,
                                 const void *Decoder) {

  unsigned rs1 = fieldFromInstruction(insn, 14, 5);
  unsigned isImm = fieldFromInstruction(insn, 13, 1);
  unsigned rs2 = 0;
  unsigned simm13 = 0;
  if (isImm)
    simm13 = SignExtend32<13>(fieldFromInstruction(insn, 0, 13));
  else
    rs2 = fieldFromInstruction(insn, 0, 5);

  // Decode RS1.
  DecodeStatus status = DecodeIntRegsRegisterClass(MI, rs1, Address, Decoder);
  if (status != MCDisassembler::Success)
    return status;

  // Decode RS2 | SIMM13.
  if (isImm)
    MI.addOperand(MCOperand::createImm(simm13));
  else {
    status = DecodeIntRegsRegisterClass(MI, rs2, Address, Decoder);
    if (status != MCDisassembler::Success)
      return status;
  }
  return MCDisassembler::Success;
}

static DecodeStatus DecodeSWAP(MCInst &MI, unsigned insn, uint64_t Address,
                               const void *Decoder) {

  unsigned rd = fieldFromInstruction(insn, 25, 5);
  unsigned rs1 = fieldFromInstruction(insn, 14, 5);
  unsigned isImm = fieldFromInstruction(insn, 13, 1);
  bool hasAsi = fieldFromInstruction(insn, 23, 1); // (in op3 field)
  unsigned asi = fieldFromInstruction(insn, 5, 8);
  unsigned rs2 = 0;
  unsigned simm13 = 0;
  if (isImm)
    simm13 = SignExtend32<13>(fieldFromInstruction(insn, 0, 13));
  else
    rs2 = fieldFromInstruction(insn, 0, 5);

  // Decode RD.
  DecodeStatus status = DecodeIntRegsRegisterClass(MI, rd, Address, Decoder);
  if (status != MCDisassembler::Success)
    return status;

  // Decode RS1.
  status = DecodeIntRegsRegisterClass(MI, rs1, Address, Decoder);
  if (status != MCDisassembler::Success)
    return status;

  // Decode RS1 | SIMM13.
  if (isImm)
    MI.addOperand(MCOperand::createImm(simm13));
  else {
    status = DecodeIntRegsRegisterClass(MI, rs2, Address, Decoder);
    if (status != MCDisassembler::Success)
      return status;
  }

  if (hasAsi)
    MI.addOperand(MCOperand::createImm(asi));

  return MCDisassembler::Success;
}

static DecodeStatus DecodeTRAP(MCInst &MI, unsigned insn, uint64_t Address,
                               const void *Decoder) {

  unsigned rs1 = fieldFromInstruction(insn, 14, 5);
  unsigned isImm = fieldFromInstruction(insn, 13, 1);
  unsigned cc =fieldFromInstruction(insn, 25, 4);
  unsigned rs2 = 0;
  unsigned imm7 = 0;
  if (isImm)
    imm7 = fieldFromInstruction(insn, 0, 7);
  else
    rs2 = fieldFromInstruction(insn, 0, 5);

  // Decode RS1.
  DecodeStatus status = DecodeIntRegsRegisterClass(MI, rs1, Address, Decoder);
  if (status != MCDisassembler::Success)
    return status;

  // Decode RS1 | IMM7.
  if (isImm)
    MI.addOperand(MCOperand::createImm(imm7));
  else {
    status = DecodeIntRegsRegisterClass(MI, rs2, Address, Decoder);
    if (status != MCDisassembler::Success)
      return status;
  }
  
  // Decode CC
  MI.addOperand(MCOperand::createImm(cc));

  return MCDisassembler::Success;
}
