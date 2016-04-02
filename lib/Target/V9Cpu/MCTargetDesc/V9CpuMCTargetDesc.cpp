
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

//@2 {
extern "C" void LLVMInitializeV9CpuTargetMC() {

}
//@2 }