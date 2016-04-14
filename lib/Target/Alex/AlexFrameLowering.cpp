#include <llvm/CodeGen/MachineFrameInfo.h>
#include <llvm/MC/MachineLocation.h>
#include <llvm/MC/MCDwarf.h>
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "AlexFrameLowering.h"
#include "AlexRegisterInfo.h"
#include "AlexInstrInfo.h"
#include "AlexTargetMachine.h"

using namespace llvm;

AlexFrameLowering::AlexFrameLowering(const AlexSubtarget *sti)
        : TargetFrameLowering(
            StackGrowsDown, AlexStackAlignment, 0, AlexStackAlignment),
            subtarget(sti) {
}

void AlexFrameLowering::determineCalleeSaves(MachineFunction &MF,
                                               BitVector &SavedRegs,
                                               RegScavenger *RS) const {
//@determineCalleeSaves-body
    TargetFrameLowering::determineCalleeSaves(MF, SavedRegs, RS);
    //Cpu0FunctionInfo *Cpu0FI = MF.getInfo<Cpu0FunctionInfo>();
    //MachineRegisterInfo& MRI = MF.getRegInfo();
   // SavedRegs.set(Alex::LR);

    return;
}

void AlexFrameLowering::emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const {
    assert(&MF.front() == &MBB && "Shrink-wrapping not yet supported");
    MachineFrameInfo *MFI    = MF.getFrameInfo();
    //Cpu0FunctionInfo *Cpu0FI = MF.getInfo<Cpu0FunctionInfo>();

    const AlexInstrInfo &TII =
            *static_cast<const AlexInstrInfo*>(subtarget->getInstrInfo());
    const AlexRegisterInfo &RegInfo =
            *static_cast<const AlexRegisterInfo *>(subtarget->getRegisterInfo());

    MachineBasicBlock::iterator MBBI = MBB.begin();
    DebugLoc dl = MBBI != MBB.end() ? MBBI->getDebugLoc() : DebugLoc();
    //Cpu0ABIInfo ABI = STI.getABI();
    unsigned SP = Alex::SP;
    const TargetRegisterClass *RC = &Alex::Int32RegsRegClass;

    // push $fp
    BuildMI(MBB, MBBI, dl, TII.get(Alex::ADDi), Alex::SP).addReg(Alex::SP).addImm(-4);
    BuildMI(MBB, MBBI, dl, TII.get(Alex::SW)).addReg(Alex::FP).addReg(Alex::SP).addImm(0);

    // move $sp, $fp
    BuildMI(MBB, MBBI, dl, TII.get(Alex::ADDi)).addReg(Alex::FP).addReg(SP).addImm(0)
            .setMIFlag(MachineInstr::FrameSetup);

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

void AlexFrameLowering::emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const {
    MachineBasicBlock::iterator MBBI = MBB.getLastNonDebugInstr();
    MachineFrameInfo *MFI            = MF.getFrameInfo();
    //Cpu0FunctionInfo *Cpu0FI = MF.getInfo<Cpu0FunctionInfo>();

    const AlexInstrInfo &TII =
            *static_cast<const AlexInstrInfo *>(subtarget->getInstrInfo());
    const AlexRegisterInfo &RegInfo =
            *static_cast<const AlexRegisterInfo *>(subtarget->getRegisterInfo());

    DebugLoc dl = MBBI->getDebugLoc();
    //Cpu0ABIInfo ABI = STI.getABI();
    unsigned SP = Alex::SP;

    // Get the number of bytes from FrameInfo
    uint64_t StackSize = MFI->getStackSize();

    if (!StackSize)
        return;

    // Adjust stack.
    TII.adjustStackPtr(SP, StackSize, MBB, MBBI);

    // restore fp
    BuildMI(MBB, MBBI, dl, TII.get(Alex::LW), Alex::FP).addReg(Alex::SP).addImm(0);
    //BuildMI(MBB, MBBI, dl, TII.get(Alex::ADDi), Alex::SP).addReg(Alex::SP).addImm(4);
    //BuildMI(MBB, MBBI, dl, TII.get(Alex::MTRA)).addReg(Alex::SP).addImm(0);
}
