#ifndef LLVM_LIB_TARGET_ALEX_ALEXINSTRINFO_H
#define LLVM_LIB_TARGET_ALEX_ALEXINSTRINFO_H

#include "AlexRegisterInfo.h"
#include "MCTargetDesc/AlexMCTargetDesc.h"
#include "llvm/Target/TargetInstrInfo.h"

#define GET_INSTRINFO_HEADER
//#define GET_INSTRINFO_ENUM
#include "AlexGenInstrInfo.inc"

namespace llvm {
  class AlexInstrInfo : public AlexGenInstrInfo {
    virtual void anchor() { }
    bool expandPostRAPseudo(MachineBasicBlock::iterator MI) const override;
    void expandRetLR(MachineBasicBlock &MBB, MachineBasicBlock::iterator I) const;
  protected:
    const AlexSubtarget *subtarget;
  public:
    explicit AlexInstrInfo(const AlexSubtarget *);

    MachineMemOperand *GetMemOperand(MachineBasicBlock &MBB, int FI,
                                                    unsigned Flag) const;

    void adjustStackPtr(unsigned SP, int64_t Amount,
                                       MachineBasicBlock &MBB,
                                       MachineBasicBlock::iterator I) const;
    void storeRegToStackSlot(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
                            unsigned SrcReg, bool isKill, int FI,
                            const TargetRegisterClass *RC, const TargetRegisterInfo *TRI) const override;
    void loadRegFromStackSlot(MachineBasicBlock &MBB, MachineBasicBlock::iterator MI,
                              unsigned DestReg, int FI, const TargetRegisterClass *RC,
                              const TargetRegisterInfo *TRI) const override;
    void copyPhysReg(MachineBasicBlock &MBB,
                     MachineBasicBlock::iterator I, DebugLoc DL,
                     unsigned DestReg, unsigned SrcReg,
                     bool KillSrc) const override;
    unsigned InsertBranch(MachineBasicBlock &MBB, MachineBasicBlock *TBB,
                                  MachineBasicBlock *FBB,
                                  ArrayRef<MachineOperand> Cond,
                                  DebugLoc DL) const override {
      return 0;
    }

  private:
    bool lowerCallPseudo(MachineBasicBlock::iterator &MI) const;
    bool lowerLoadExtendPseudo(MachineBasicBlock::iterator &MI) const;
    bool lowerSExtPseudo(MachineBasicBlock::iterator &MI) const;

    bool lowerSelect(MachineBasicBlock::iterator &MI) const;
    bool lowerArithLogicRRR(MachineBasicBlock::iterator &MI, unsigned Opcode) const;
    bool lowerArithLogicRRR(MachineBasicBlock::iterator &MI, unsigned Opcode,
                            unsigned resultReg, unsigned lhsReg, unsigned rhsReg) const;
    bool lowerArithLogicRRI(MachineBasicBlock::iterator &MI, unsigned Opcode,
                            unsigned resultReg, unsigned srcReg, unsigned Imm) const;

    bool lowerV9Cmp(MachineBasicBlock::iterator &MI, unsigned Opcode) const;
    bool lowerV9Branch(MachineBasicBlock::iterator &MI, unsigned Opcode) const;
    bool lowerAbsoluteCall(MachineBasicBlock::iterator &MI, unsigned Reg) const;

    bool lowerLoadFI(MachineBasicBlock::iterator &MI, int32_t FI, unsigned Reg) const;
    bool lowerStoreFI(MachineBasicBlock::iterator &MI, int32_t FI, unsigned Reg) const;
    bool lowerLoadMem(MachineBasicBlock::iterator &MI, unsigned Dst, unsigned Base, int Offset) const;
    bool lowerStoreMem(MachineBasicBlock::iterator &MI, unsigned Src, unsigned Base, int Offset) const;

    bool lowerPushR(MachineBasicBlock::iterator &MI, unsigned Reg) const;
    bool lowerPushI(MachineBasicBlock::iterator &MI, unsigned Imm16, unsigned Flags = 0) const;
    bool lowerPopR(MachineBasicBlock::iterator &MI, unsigned Reg) const;
    bool lowerPush(MachineBasicBlock::iterator &MI, unsigned Reg) const;
    bool lowerPop(MachineBasicBlock::iterator &MI, unsigned Reg) const;
    bool lowerMov(MachineBasicBlock::iterator &MI, unsigned Dest, unsigned Src) const;

    bool lowerLI24(MachineBasicBlock::iterator &MI, unsigned Reg, uint32_t Imm) const;
    bool lowerLI24(MachineBasicBlock::iterator &MI, unsigned Reg, const GlobalValue *GlobalAddr) const;
    bool lowerLIH8(MachineBasicBlock::iterator &MI, unsigned Reg, uint32_t Imm) const;
    bool lowerLIH8(MachineBasicBlock::iterator &MI, unsigned Reg, const GlobalValue *GlobalAddr) const;
    bool lowerLI32(MachineBasicBlock::iterator &MI, unsigned Reg, uint32_t Imm) const;
    bool lowerLI32(MachineBasicBlock::iterator &MI, unsigned Reg, const GlobalValue *GlobalAddr) const;
  };
}

#endif