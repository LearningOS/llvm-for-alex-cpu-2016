#include "V9CpuFrameLowering.h"
#include "V9CpuSubtarget.h"
#include "V9CpuRegisterInfo.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Target/TargetFrameLowering.h"

using namespace llvm;

const V9CpuFrameLowering *V9CpuFrameLowering::create(const V9CpuSubtarget &ST) {
    return llvm::createV9CpuSEFrameLowering(ST);
}

// hasFP - Return true if the specified function should have a dedicated frame
// pointer register.  This is true if the function has variable sized allocas,
// if it needs dynamic stack realignment, if frame pointer elimination is
// disabled, or if the frame address is taken.
bool V9CpuFrameLowering::hasFP(const MachineFunction &MF) const {
    const MachineFrameInfo *MFI = MF.getFrameInfo();
    const TargetRegisterInfo *TRI = STI.getRegisterInfo();

    return MF.getTarget().Options.DisableFramePointerElim(MF) ||
           MFI->hasVarSizedObjects() || MFI->isFrameAddressTaken() ||
           TRI->needsStackRealignment(MF);
}