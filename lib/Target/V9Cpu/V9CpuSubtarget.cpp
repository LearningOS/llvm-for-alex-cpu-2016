
#include "V9CpuSubtarget.h"

#include "V9Cpu.h"
#include "V9CpuRegisterInfo.h"

#include "V9CpuTargetMachine.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

#define DEBUG_TYPE "V9Cpu-subtarget"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "V9CpuGenSubtargetInfo.inc"

extern bool FixGlobalBaseReg;

/// Select the V9Cpu CPU for the given triple and cpu name.
/// FIXME: Merge with the copy in V9CpuMCTargetDesc.cpp
static StringRef selectV9CpuCPU(Triple TT, StringRef CPU) {
    if (CPU.empty() || CPU == "v9cpu-generic")
        CPU = "V9CpuI";
    return CPU;
}

void V9CpuSubtarget::anchor() { }

//@1 {
V9CpuSubtarget::V9CpuSubtarget(const Triple &TT, const std::string &CPU,
                             const std::string &FS, bool little,
                             const V9CpuTargetMachine &_TM) :
//@1 }
// V9CpuGenSubtargetInfo will display features by llc -march=V9Cpu -mcpu=help
        V9CpuGenSubtargetInfo(TT, CPU, FS),
        TM(_TM), TargetTriple(TT),
        InstrInfo(V9CpuInstrInfo::create(initializeSubtargetDependencies(CPU, FS, TM))),
        FrameLowering(V9CpuFrameLowering::create(*this)),
        TLInfo(V9CpuTargetLowering::create(TM, *this)) {

}

V9CpuSubtarget &
V9CpuSubtarget::initializeSubtargetDependencies(StringRef CPU, StringRef FS,
                                               const TargetMachine &TM) {
    std::string CPUName = selectV9CpuCPU(TargetTriple, CPU);

    if (CPUName == "help")
        CPUName = "V9CpuI";

    V9CpuArchVersion = V9CpuI;

    // Parse features string.
    ParseSubtargetFeatures(CPUName, FS);
    // Initialize scheduling itinerary for the specified CPU.
    InstrItins = getInstrItineraryForCPU(CPUName);

    return *this;
}


const V9CpuABIInfo &V9CpuSubtarget::getABI() const { return TM.getABI(); }