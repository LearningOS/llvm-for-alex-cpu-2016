#include "InstPrinter/V9CpuInstPrinter.h"
#include "V9CpuMCAsmInfo.h"
#include "V9CpuMCTargetDesc.h"
#include "V9CpuFrameLowering.h"
#include "llvm/MC/MachineLocation.h"
#include "llvm/MC/MCCodeGenInfo.h"
#include "llvm/MC/MCELFStreamer.h"
#include "llvm/MC/MCInstPrinter.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;

#define GET_INSTRINFO_MC_DESC
#include "V9CpuGenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "V9CpuGenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "V9CpuGenRegisterInfo.inc"
/// Select the V9Cpu Architecture Feature for the given triple and cpu name.
/// The function will be called at command 'llvm-objdump -d' for V9Cpu elf input.
static StringRef selectV9CpuArchFeature(const Triple &TT, StringRef CPU) {
  std::string V9CpuArchFeature;
  if (CPU.empty() || CPU == "v9-generic") {
    V9CpuArchFeature = "+v9cpui";
  }
  return V9CpuArchFeature;
}
//@1 }

static MCInstrInfo *createV9CpuMCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitV9CpuMCInstrInfo(X); // defined in V9CpuGenInstrInfo.inc
  return X;
}

static MCRegisterInfo *createV9CpuMCRegisterInfo(const Triple &TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitV9CpuMCRegisterInfo(X, V9Cpu::B); // defined in V9CpuGenRegisterInfo.inc
  return X;
}

static MCSubtargetInfo *createV9CpuMCSubtargetInfo(const Triple &TT,
                                                  StringRef CPU, StringRef FS) {
  std::string ArchFS = selectV9CpuArchFeature(TT,CPU);
  if (!FS.empty()) {
    if (!ArchFS.empty())
      ArchFS = ArchFS + "," + FS.str();
    else
      ArchFS = FS;
  }
  return createV9CpuMCSubtargetInfoImpl(TT, CPU, ArchFS);
}

static MCCodeGenInfo *createV9CpuMCCodeGenInfo(const Triple &TT, Reloc::Model RM,
                                              CodeModel::Model CM,
                                              CodeGenOpt::Level OL) {
  MCCodeGenInfo *X = new MCCodeGenInfo();
  if (CM == CodeModel::JITDefault)
    RM = Reloc::Static;
  else if (RM == Reloc::Default)
    RM = Reloc::PIC_;
  X->initMCCodeGenInfo(RM, CM, OL); // defined in lib/MC/MCCodeGenInfo.cpp
  return X;
}

MCAsmInfo *createV9CpuMCAsmInfo(const MCRegisterInfo &MRI,
                                const Triple &TT) {
  MCAsmInfo *MAI = new V9CpuMCAsmInfo(TT);

  unsigned SP = MRI.getDwarfRegNum(V9Cpu::SP, true);
  MCCFIInstruction Inst = MCCFIInstruction::createDefCfa(nullptr, SP, 0);
  MAI->addInitialFrameState(Inst);

  return MAI;
}

static MCInstPrinter *createV9CpuMCInstPrinter(const Triple &T,
                                              unsigned SyntaxVariant,
                                              const MCAsmInfo &MAI,
                                              const MCInstrInfo &MII,
                                              const MCRegisterInfo &MRI) {
  return new V9CpuInstPrinter(MAI, MII, MRI);
}

//@2 {
extern "C" void LLVMInitializeV9CpuTargetMC() {
  for (Target *T : {&TheV9CpuTarget, &TheV9CpuTarget}) {
    // Register the MC asm info.
    RegisterMCAsmInfoFn X(*T, createV9CpuMCAsmInfo);

    // Register the MC codegen info.
    TargetRegistry::RegisterMCCodeGenInfo(*T,
                                          createV9CpuMCCodeGenInfo);

    // Register the MC instruction info.
    TargetRegistry::RegisterMCInstrInfo(*T, createV9CpuMCInstrInfo);

    // Register the MC register info.
    TargetRegistry::RegisterMCRegInfo(*T, createV9CpuMCRegisterInfo);

    // Register the MC subtarget info.
    TargetRegistry::RegisterMCSubtargetInfo(*T,
                                            createV9CpuMCSubtargetInfo);
    // Register the MCInstPrinter.
    TargetRegistry::RegisterMCInstPrinter(*T,
                                          createV9CpuMCInstPrinter);
  }

}
//@2 }