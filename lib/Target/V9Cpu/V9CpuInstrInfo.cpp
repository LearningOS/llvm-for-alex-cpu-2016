#include "V9CpuInstrInfo.h"

#include "V9CpuTargetMachine.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

#define GET_INSTRINFO_CTOR_DTOR
#include "V9CpuGenInstrInfo.inc"

// Pin the vtable to this file.
void V9CpuInstrInfo::anchor() {}

//@V9CpuInstrInfo {
V9CpuInstrInfo::V9CpuInstrInfo(const V9CpuSubtarget &STI)
        :
        Subtarget(STI) {}

const V9CpuInstrInfo *V9CpuInstrInfo::create(V9CpuSubtarget &STI) {
    return llvm::createV9CpuSEInstrInfo(STI);
}

MachineInstr*
V9CpuInstrInfo::emitFrameIndexDebugValue(MachineFunction &MF, int FrameIx,
                                        uint64_t Offset, const MDNode *MDPtr,
                                        DebugLoc DL) const {
  MachineInstrBuilder MIB = BuildMI(MF, DL, get(V9Cpu::DBG_VALUE))
    .addFrameIndex(FrameIx).addImm(0).addImm(Offset).addMetadata(MDPtr);
  return &*MIB;
}

MachineMemOperand *V9CpuInstrInfo::GetMemOperand(MachineBasicBlock &MBB, int FI,
                                                unsigned Flag) const {
    MachineFunction &MF = *MBB.getParent();
    MachineFrameInfo &MFI = *MF.getFrameInfo();
    unsigned Align = MFI.getObjectAlignment(FI);

    return MF.getMachineMemOperand(MachinePointerInfo::getFixedStack(MF, FI, 0), Flag,
                                   MFI.getObjectSize(FI), Align);
}