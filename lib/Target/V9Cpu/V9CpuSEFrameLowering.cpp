#include "V9CpuSEFrameLowering.h"

#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/RegisterScavenging.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Target/TargetFrameLowering.h"

using namespace llvm;

V9CpuSEFrameLowering::V9CpuSEFrameLowering(const V9CpuSubtarget &STI)
        : V9CpuFrameLowering(STI, STI.stackAlignment()) {}

//@emitPrologue {
void V9CpuSEFrameLowering::emitPrologue(MachineFunction &MF,
                                       MachineBasicBlock &MBB) const {
}
//}

//@emitEpilogue {
void V9CpuSEFrameLowering::emitEpilogue(MachineFunction &MF,
                                       MachineBasicBlock &MBB) const {
}
//}

const V9CpuFrameLowering *
llvm::createV9CpuSEFrameLowering(const V9CpuSubtarget &ST) {
    return new V9CpuSEFrameLowering(ST);
}