#include <AlexTargetMachine.h>
#include "AlexFrameLowering.h"
#include "InstPrinter/AlexInstPrinter.h"
#include "AlexMCAsmInfo.h"
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

#define GET_REGINFO_MC_DESC
#include "AlexGenRegisterInfo.inc"

#define GET_INSTRINFO_MC_DESC
#include "AlexGenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "AlexGenSubtargetInfo.inc"

/// Select the Alex Architecture Feature for the given triple and cpu name.
/// The function will be called at command 'llvm-objdump -d' for Alex elf input.
static StringRef selectAlexArchFeature(const Triple &TT, StringRef CPU) {
    std::string AlexArchFeature;
    if (CPU.empty() || CPU == "a1") {
        AlexArchFeature = "+a1";
    }
    return AlexArchFeature;
}
//@1 }

static MCInstrInfo *createAlexMCInstrInfo() {
    MCInstrInfo *X = new MCInstrInfo();
    InitAlexMCInstrInfo(X); // defined in AlexGenInstrInfo.inc
    return X;
}

static MCRegisterInfo *createAlexMCRegisterInfo(const Triple &TT) {
    MCRegisterInfo *X = new MCRegisterInfo();
    InitAlexMCRegisterInfo(X, Alex::R1); // defined in AlexGenRegisterInfo.inc
    return X;
}

static MCSubtargetInfo *createAlexMCSubtargetInfo(const Triple &TT,
                                                   StringRef CPU, StringRef FS) {
    std::string ArchFS = selectAlexArchFeature(TT,CPU);
    if (!FS.empty()) {
        if (!ArchFS.empty())
            ArchFS = ArchFS + "," + FS.str();
        else
            ArchFS = FS;
    }
    return createAlexMCSubtargetInfoImpl(TT, CPU, ArchFS);
}

static MCCodeGenInfo *createAlexMCCodeGenInfo(const Triple &TT, Reloc::Model RM,
                                               CodeModel::Model CM,
                                               CodeGenOpt::Level OL) {
    MCCodeGenInfo *X = new MCCodeGenInfo();
//  if (CM == CodeModel::JITDefault)
//    RM = Reloc::Static;
//  else if (RM == Reloc::Default)
//    RM = Reloc::PIC_;
    X->initMCCodeGenInfo(RM, CM, OL); // defined in lib/MC/MCCodeGenInfo.cpp
    return X;
}

MCAsmInfo *createAlexMCAsmInfo(const MCRegisterInfo &MRI,
                                const Triple &TT) {
    MCAsmInfo *MAI = new AlexMCAsmInfo(TT);

    unsigned SP = MRI.getDwarfRegNum(Alex::SP, true);
    MCCFIInstruction Inst = MCCFIInstruction::createDefCfa(nullptr, SP, 0);
    MAI->addInitialFrameState(Inst);

    return MAI;
}

static MCInstPrinter *createAlexMCInstPrinter(const Triple &T,
                                               unsigned SyntaxVariant,
                                               const MCAsmInfo &MAI,
                                               const MCInstrInfo &MII,
                                               const MCRegisterInfo &MRI) {
    return new AlexInstPrinter(MAI, MII, MRI);
}

//@2 {
extern "C" void LLVMInitializeAlexTargetMC() {
    for (Target *T : {&TheAlexTarget, &TheAlexTarget}) {
        // Register the MC asm info.
        RegisterMCAsmInfoFn X(*T, createAlexMCAsmInfo);

        // Register the MC codegen info.
        TargetRegistry::RegisterMCCodeGenInfo(*T,
                                              createAlexMCCodeGenInfo);

        // Register the MC instruction info.
        TargetRegistry::RegisterMCInstrInfo(*T, createAlexMCInstrInfo);

        // Register the MC register info.
        TargetRegistry::RegisterMCRegInfo(*T, createAlexMCRegisterInfo);

        // Register the MC subtarget info.
        TargetRegistry::RegisterMCSubtargetInfo(*T,
                                                createAlexMCSubtargetInfo);
        // Register the MCInstPrinter.
        TargetRegistry::RegisterMCInstPrinter(*T,
                                              createAlexMCInstPrinter);
    }

}