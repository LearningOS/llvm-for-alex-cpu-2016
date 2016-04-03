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
#include "V9CpuSEInstrInfo.h"

using namespace llvm;

V9CpuSEFrameLowering::V9CpuSEFrameLowering(const V9CpuSubtarget &STI)
        : V9CpuFrameLowering(STI, STI.stackAlignment()) {}

const V9CpuFrameLowering *
llvm::createV9CpuSEFrameLowering(const V9CpuSubtarget &ST) {
    return new V9CpuSEFrameLowering(ST);
}

void V9CpuSEFrameLowering::emitPrologue(MachineFunction &MF,
                                       MachineBasicBlock &MBB) const {
    assert(&MF.front() == &MBB && "Shrink-wrapping not yet supported");
    MachineFrameInfo *MFI    = MF.getFrameInfo();
    V9CpuFunctionInfo *V9CpuFI = MF.getInfo<V9CpuFunctionInfo>();

    const V9CpuSEInstrInfo &TII =
            *static_cast<const V9CpuSEInstrInfo*>(STI.getInstrInfo());
    const V9CpuRegisterInfo &RegInfo =
            *static_cast<const V9CpuRegisterInfo *>(STI.getRegisterInfo());

    MachineBasicBlock::iterator MBBI = MBB.begin();
    DebugLoc dl = MBBI != MBB.end() ? MBBI->getDebugLoc() : DebugLoc();
    V9CpuABIInfo ABI = STI.getABI();
    unsigned SP = V9Cpu::SP;
    const TargetRegisterClass *RC = &V9Cpu::Int32RegsRegClass;

    // First, compute final stack size.
    uint64_t StackSize = MFI->getStackSize();

    // No need to allocate space on the stack.
    if (StackSize == 0 && !MFI->adjustsStack()) return;

    MachineModuleInfo &MMI = MF.getMMI();
    const MCRegisterInfo *MRI = MMI.getContext().getRegisterInfo();
    MachineLocation DstML, SrcML;

    // Adjust stack.
    TII.adjustStackPtr(SP, -StackSize, MBB, MBBI);

    // emit ".cfi_def_cfa_offset StackSize"
    unsigned CFIIndex = MMI.addFrameInst(
            MCCFIInstruction::createDefCfaOffset(nullptr, -StackSize));
    BuildMI(MBB, MBBI, dl, TII.get(TargetOpcode::CFI_INSTRUCTION))
            .addCFIIndex(CFIIndex);

    const std::vector<CalleeSavedInfo> &CSI = MFI->getCalleeSavedInfo();

    if (CSI.size()) {
        // Find the instruction past the last instruction that saves a callee-saved
        // register to the stack.
        for (unsigned i = 0; i < CSI.size(); ++i)
            ++MBBI;

        // Iterate over list of callee-saved registers and emit .cfi_offset
        // directives.
        for (std::vector<CalleeSavedInfo>::const_iterator I = CSI.begin(),
                     E = CSI.end(); I != E; ++I) {
            int64_t Offset = MFI->getObjectOffset(I->getFrameIdx());
            unsigned Reg = I->getReg();
            {
                // Reg is in CPURegs.
                unsigned CFIIndex = MMI.addFrameInst(MCCFIInstruction::createOffset(
                        nullptr, MRI->getDwarfRegNum(Reg, 1), Offset));
                BuildMI(MBB, MBBI, dl, TII.get(TargetOpcode::CFI_INSTRUCTION))
                        .addCFIIndex(CFIIndex);
            }
        }
    }

}

void V9CpuSEFrameLowering::emitEpilogue(MachineFunction &MF,
                                       MachineBasicBlock &MBB) const {
    MachineBasicBlock::iterator MBBI = MBB.getLastNonDebugInstr();
    MachineFrameInfo *MFI            = MF.getFrameInfo();
    V9CpuFunctionInfo *V9CpuFI = MF.getInfo<V9CpuFunctionInfo>();

    const V9CpuSEInstrInfo &TII =
            *static_cast<const V9CpuSEInstrInfo *>(STI.getInstrInfo());
    const V9CpuRegisterInfo &RegInfo =
            *static_cast<const V9CpuRegisterInfo *>(STI.getRegisterInfo());

    DebugLoc dl = MBBI->getDebugLoc();
    V9CpuABIInfo ABI = STI.getABI();
    unsigned SP = V9Cpu::SP;

    // Get the number of bytes from FrameInfo
    uint64_t StackSize = MFI->getStackSize();

    if (!StackSize)
        return;

    // Adjust stack.
    TII.adjustStackPtr(SP, StackSize, MBB, MBBI);
}
bool
V9CpuSEFrameLowering::hasReservedCallFrame(const MachineFunction &MF) const {
    const MachineFrameInfo *MFI = MF.getFrameInfo();

    // Reserve call frame if the size of the maximum call frame fits into 16-bit
    // immediate field and there are no variable sized objects on the stack.
    // Make sure the second register scavenger spill slot can be accessed with one
    // instruction.
    return isInt<16>(MFI->getMaxCallFrameSize() + getStackAlignment()) &&
           !MFI->hasVarSizedObjects();
}

void V9CpuSEFrameLowering::determineCalleeSaves(MachineFunction &MF,
                                               BitVector &SavedRegs,
                                               RegScavenger *RS) const {
//@determineCalleeSaves-body
    TargetFrameLowering::determineCalleeSaves(MF, SavedRegs, RS);
    V9CpuFunctionInfo *V9CpuFI = MF.getInfo<V9CpuFunctionInfo>();
    MachineRegisterInfo& MRI = MF.getRegInfo();

    //if (MF.getFrameInfo()->hasCalls())
   //     setAliasRegs(MF, SavedRegs, V9Cpu::A);

    return;
}