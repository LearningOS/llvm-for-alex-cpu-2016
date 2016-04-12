
#include "AlexInstrInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineMemOperand.h"

using namespace llvm;

class AlexSubtarget;

#define GET_INSTRINFO_CTOR_DTOR
#include "AlexGenInstrInfo.inc"

AlexInstrInfo::AlexInstrInfo(const AlexSubtarget *subtarget)
        :subtarget(subtarget) {

}

void AlexInstrInfo::adjustStackPtr(unsigned SP, int64_t Amount,
                                     MachineBasicBlock &MBB,
                                     MachineBasicBlock::iterator I) const {
    DebugLoc DL = I != MBB.end() ? I->getDebugLoc() : DebugLoc();
    unsigned ADDiu = Alex::ADDiu;
    BuildMI(MBB, I, DL, get(ADDiu), SP).addReg(SP).addImm(Amount);
}

MachineMemOperand *AlexInstrInfo::GetMemOperand(MachineBasicBlock &MBB, int FI,
                                                unsigned Flag) const {
    MachineFunction &MF = *MBB.getParent();
    MachineFrameInfo &MFI = *MF.getFrameInfo();
    unsigned Align = MFI.getObjectAlignment(FI);

    return MF.getMachineMemOperand(MachinePointerInfo::getFixedStack(FI), Flag,
                                   MFI.getObjectSize(FI), Align);
}


void AlexInstrInfo::storeRegToStackSlot(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
                unsigned SrcReg, bool isKill, int FI,
                const TargetRegisterClass *RC, const TargetRegisterInfo *TRI) const {
    DebugLoc DL;
    if (I != MBB.end()) DL = I->getDebugLoc();
    MachineMemOperand *MMO = GetMemOperand(MBB, FI, MachineMemOperand::MOStore);

    unsigned Opc = 0;

    Opc = Alex::FP;
    assert(Opc && "Register class not handled!");
    BuildMI(MBB, I, DL, get(Opc)).addReg(SrcReg, getKillRegState(isKill))
            .addFrameIndex(FI).addImm(0).addMemOperand(MMO);
}
