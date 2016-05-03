#ifndef ALEX_TARGET_MACHINE_H
#define ALEX_TARGET_MACHINE_H

#include "llvm/Target/TargetMachine.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/Target/TargetFrameLowering.h"
#include "AlexInstrInfo.h"
#include "AlexFrameLowering.h"
#include "AlexISelLowering.h"

#define GET_SUBTARGETINFO_HEADER
#include "AlexGenSubtargetInfo.inc"
namespace llvm {
extern Target TheAlexTarget;

class StringRef;

class AlexTargetMachine;

class AlexRegisterInfo;

class AlexTargetLowering;

class AlexFrameLowering;

class AlexRegisterInfo;

class SelectionDAGTargetInfo;

class AlexSubtarget : public AlexGenSubtargetInfo {
public:
  AlexSubtarget(const Triple &TT, const std::string &CPU, const std::string &FS,
                const AlexTargetMachine *_TM);

  const AlexInstrInfo *getInstrInfo() const override {
    return instrInfo;
  }

  const AlexFrameLowering *getFrameLowering() const override {
    return FrameLowering;
  }

  const AlexRegisterInfo *getRegisterInfo() const override {
    return registerInfo;
  }

  const AlexTargetLowering *getTargetLowering() const override {
    return targetLowering;
  }

  const InstrItineraryData *getInstrItineraryData() const override {
    return instrItineraryData;
  }

  const SelectionDAGTargetInfo *getSelectionDAGInfo() const override {
    return TSInfo;
  }

  // generate by td
  void ParseSubtargetFeatures(StringRef CPU, StringRef FS);

private:
  virtual void anchor() { }

protected:
  Triple targetTriple;

  const AlexTargetMachine *targetMachine;
  const AlexInstrInfo *instrInfo;
  const AlexFrameLowering *FrameLowering;
  InstrItineraryData *instrItineraryData;
  const SelectionDAGTargetInfo *TSInfo;
  const AlexRegisterInfo *registerInfo;
  const AlexTargetLowering *targetLowering;
};

class AlexTargetMachine : public LLVMTargetMachine {

public:
  AlexTargetMachine(const Target &target, const Triple &targetTripple, StringRef cpu,
                    StringRef fs, const TargetOptions &options,
                    Reloc::Model RM, CodeModel::Model CM, CodeGenOpt::Level OL);

  ~AlexTargetMachine() override { }

  const AlexSubtarget *getSubtargetImpl(const Function &F) const override {
    return getSubtargetImpl();
  }

  // for asm printer
  const AlexSubtarget *getSubtargetImpl() const {
    return defaultSubtarget;
  }

  TargetPassConfig *createPassConfig(PassManagerBase &PM) override;

  TargetLoweringObjectFile *getObjFileLowering() const override {
    return targetLoweringObjectFile;
  }

private:
  AlexSubtarget *defaultSubtarget;
  TargetLoweringObjectFile *targetLoweringObjectFile;
};

} // end namespace llvm


#endif // ALEX_TARGET_MACHINE_H