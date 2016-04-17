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
    unsigned ADDi = Alex::ADDi;
    BuildMI(MBB, I, DL, get(ADDi), SP).addReg(SP).addImm(Amount);
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
   // BuildMI(MBB, I, I->getDebugLoc(), get(Alex::JRRA)).addReg(Alex::RA);
    BuildMI(MBB, I, I->getDebugLoc(), get(Alex::RET));
}
bool AlexInstrInfo::expandPostRAPseudo(MachineBasicBlock::iterator MI) const {
//@expandPostRAPseudo-body
    MachineBasicBlock &MBB = *MI->getParent();

    int instructionsAfterSavedPC = 1; // 从t0 = pc+xx到if之间一共有1条指令
    switch(MI->getDesc().getOpcode()) {
        default:
            return false;
        case Alex::RetLR:
            expandRetLR(MBB, MI);
            break;
        case Alex::LI32: {
            auto reg = MI->getOperand(0).getReg();
            auto immOprand = MI->getOperand(1);
            if (immOprand.isImm()) {
                auto val = immOprand.getImm();
                BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::LI), reg).addImm(val & 0xFFFF);
                BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::LIH), reg).addImm((val >> 16) & 0xFFFF);
            }
            else {
                auto globalAddr = immOprand.getGlobal();
                BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::LI), reg).addImm(0);
                BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::LIH), reg).addImm(0);
            }

            break;
        }

        case Alex::CALLr:
            instructionsAfterSavedPC += 1;
        case Alex::CALLi:
            instructionsAfterSavedPC += 2;
        case Alex::CALLg:
            instructionsAfterSavedPC += 1;
            // save t0, t1, t2
            BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::ADDi), Alex::SP).addReg(Alex::SP).addImm(-4*4);
            BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::SW)).addReg(Alex::T0).addReg(Alex::SP).addImm(4);
            BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::SW)).addReg(Alex::T1).addReg(Alex::SP).addImm(8);
            BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::SW)).addReg(Alex::T2).addReg(Alex::SP).addImm(12);

            // push ($pc+xx)  # ret address
            BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::ADDi), Alex::T0).addReg(Alex::PC).addImm(instructionsAfterSavedPC*4);
            BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::SW)).addReg(Alex::T0).addReg(Alex::SP).addImm(0);

            // call func
            if (MI->getDesc().getOpcode() == Alex::CALLg) {
                BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::CALL)).addGlobalAddress(MI->getOperand(0).getGlobal());
            }
            else if (MI->getDesc().getOpcode() == Alex::CALLi) {
                auto imm = MI->getOperand(0).getImm();
                BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::LI), Alex::T0).addImm(imm & 0xFFFF);
                BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::LIH), Alex::T0).addImm((imm>>16) & 0xFFFF);
                BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::CALL)).addReg(Alex::T0);
            }
            else {
                // temporary restore t0, t1, t2 in case that it uses t0~t2 as call addr
                BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::LW), Alex::T0).addReg(Alex::SP).addImm(4);
                BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::LW), Alex::T1).addReg(Alex::SP).addImm(8);
                BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::LW), Alex::T2).addReg(Alex::SP).addImm(12);
                BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::CALL)).addReg(MI->getOperand(0).getReg());
            }

            // restore t0, t1, t2
            BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::LW), Alex::T0).addReg(Alex::SP).addImm(0);
            BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::LW), Alex::T1).addReg(Alex::SP).addImm(4);
            BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::LW), Alex::T2).addReg(Alex::SP).addImm(8);
            BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::ADDi), Alex::SP).addReg(Alex::SP).addImm(4*3);
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

void AlexInstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                                  MachineBasicBlock::iterator I, DebugLoc DL,
                                  unsigned DestReg, unsigned SrcReg,
                                  bool KillSrc) const {
    unsigned Opc = 0;

    if (Alex::Int32RegsRegClass.contains(DestReg)) { // Copy to CPU Reg.
        if (Alex::Int32RegsRegClass.contains(SrcReg))
            Opc = Alex::ADDi;
    }

    assert(Opc  && DestReg && SrcReg && "Cannot copy registers");

    BuildMI(MBB, I, DL, get(Opc), DestReg).
                    addReg(SrcReg, getKillRegState(KillSrc)).
                    addImm(0);
}