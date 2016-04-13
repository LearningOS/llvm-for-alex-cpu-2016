#include <llvm/CodeGen/MachineMemOperand.h>
#include "AlexInstrInfo.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/TargetRegistry.h"

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

    return MF.getMachineMemOperand(MachinePointerInfo::getFixedStack(MF, FI, 0), Flag,
                                   MFI.getObjectSize(FI), Align);
}

void AlexInstrInfo::expandRetLR(MachineBasicBlock &MBB,
                                  MachineBasicBlock::iterator I) const {
    //loadRegFromStackSlot(MBB, I, Alex::LR, 0, nullptr, nullptr);

    //BuildMI(MBB, I, I->getDebugLoc(), get(Alex::LW)).addReg(Alex::RA).addReg(Alex::FP).addImm(0);
    BuildMI(MBB, I, I->getDebugLoc(), get(Alex::JRRA)).addReg(Alex::RA);
}
bool AlexInstrInfo::expandPostRAPseudo(MachineBasicBlock::iterator MI) const {
//@expandPostRAPseudo-body
    MachineBasicBlock &MBB = *MI->getParent();

    switch(MI->getDesc().getOpcode()) {
        default:
            return false;
        case Alex::RetLR:
            expandRetLR(MBB, MI);
            break;
        case Alex::Call:
            // push $pc
            //MI->getOperand(1).getGlobal();
            BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::ADDiu), Alex::SP).addReg(Alex::SP).addImm(-4);
            BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::SW)).addReg(Alex::R1).addReg(Alex::SP).addImm(0);
//            // addiu $r1, $pc, 12
            BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::ADDiu),Alex::R1).addReg(Alex::PC).addImm(24);
//            // push $r1
            BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::ADDiu), Alex::SP).addReg(Alex::SP).addImm(-4);
            BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::SW)).addReg(Alex::R1).addReg(Alex::SP).addImm(0);
//            // lw $r1, 4($sp)
            BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::LW), Alex::R1).addReg(Alex::SP).addImm(-4);
            // j func
            BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::J)).addGlobalAddress(MI->getOperand(0).getGlobal());
            BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::ADDiu), Alex::SP).addReg(Alex::SP).addImm(8);
            break;
    }

    MBB.erase(MI);
    return true;
}

void AlexInstrInfo::storeRegToStackSlot(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
                unsigned SrcReg, bool isKill, int FI,
                const TargetRegisterClass *RC, const TargetRegisterInfo *TRI) const {
    DebugLoc DL;
    if (I != MBB.end()) DL = I->getDebugLoc();
    MachineMemOperand *MMO = GetMemOperand(MBB, FI, MachineMemOperand::MOStore);

    unsigned Opc = 0;

    Opc = Alex::SW;
    assert(Opc && "Register class not handled!");
    BuildMI(MBB, I, DL, get(Opc)).addReg(SrcReg, getKillRegState(isKill))
            .addFrameIndex(FI).addImm(0).addMemOperand(MMO);
}


void AlexInstrInfo::loadRegFromStackSlot(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
                 unsigned DestReg, int FI, const TargetRegisterClass *RC,
                 const TargetRegisterInfo *TRI) const {
  DebugLoc DL;
  if (I != MBB.end()) DL = I->getDebugLoc();
  MachineMemOperand *MMO = GetMemOperand(MBB, FI, MachineMemOperand::MOLoad);
  unsigned Opc = 0;

  Opc = Alex::LW;
  assert(Opc && "Register class not handled!");
  BuildMI(MBB, I, DL, get(Opc), DestReg).addFrameIndex(FI).addImm(0)
          .addMemOperand(MMO);
}