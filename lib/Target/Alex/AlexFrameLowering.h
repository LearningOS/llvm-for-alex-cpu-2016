#ifndef LLVM_LIB_TARGET_ALEX_ALEXFRAMELOWERING_H
#define LLVM_LIB_TARGET_ALEX_ALEXFRAMELOWERING_H

#include "llvm/Target/TargetFrameLowering.h"

constexpr int AlexStackAlignment = 8;

namespace llvm {
  class AlexSubtarget;
  class AlexFrameLowering : public TargetFrameLowering {
  protected:
    const AlexSubtarget *subtarget;
  public:
    explicit AlexFrameLowering(const AlexSubtarget *sti);
    virtual void emitPrologue(MachineFunction &MF,
                              MachineBasicBlock &MBB) const override;
    virtual void emitEpilogue(MachineFunction &MF,
                              MachineBasicBlock &MBB) const override;

    bool hasFP(const MachineFunction &MF) const override {
      //return true;
      return false;
    }
    void determineCalleeSaves(MachineFunction &MF,
                                                 BitVector &SavedRegs,
                                                 RegScavenger *RS) const override;
    bool spillCalleeSavedRegisters(MachineBasicBlock &MBB,
                                    MachineBasicBlock::iterator MI,
                                    const std::vector<CalleeSavedInfo> &CSI,
                                    const TargetRegisterInfo *TRI) const override;
  };
} // End llvm namespace

#endif