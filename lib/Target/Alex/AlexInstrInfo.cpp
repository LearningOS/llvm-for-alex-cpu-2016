#include <llvm/CodeGen/MachineMemOperand.h>
#include <MCTargetDesc/AlexBaseInfo.h>
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
            BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::LIh), reg).addImm((val >> 16) & 0xFFFF);
        }
        else {
            auto globalAddr = immOprand.getGlobal();
            BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::LI), reg).addImm(0);
            BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::LIh), reg).addImm(0);
        }

        break;
    }

    case Alex::CALLr:
    case Alex::CALLg:
        // save t0, t1, t2
//        BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::ADDi), Alex::SP).addReg(Alex::SP).addImm(-4*5);
//        BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::SW)).addReg(Alex::T0).addReg(Alex::SP).addImm(4);
//        BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::SW)).addReg(Alex::T1).addReg(Alex::SP).addImm(8);
//        BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::SW)).addReg(Alex::T2).addReg(Alex::SP).addImm(12);
//        BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::SW)).addReg(Alex::T3).addReg(Alex::SP).addImm(16);
//        BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::SW)).addReg(Alex::T4).addReg(Alex::SP).addImm(20);

        // call func
        if (MI->getDesc().getOpcode() == Alex::CALLg) {
            // 这里不用备份T0, 由于T0用于返回地址, 此处LLVM会标记T0为LiveIn
            BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::LIh), Alex::T0).addGlobalAddress(MI->getOperand(0).getGlobal(), AlexII::MO_ABS_HI);
            BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::ADDiu), Alex::T0).addReg(Alex::T0).addGlobalAddress(MI->getOperand(0).getGlobal(), AlexII::MO_ABS_LO);
            BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::CALL)).addReg(Alex::T0);
        }
        else {
            // temporary restore t0, t1, t2 in case that it uses t0~t2 as call addr
//            BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::LW), Alex::T0).addReg(Alex::SP).addImm(0);
//            BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::LW), Alex::T1).addReg(Alex::SP).addImm(4);
//            BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::LW), Alex::T2).addReg(Alex::SP).addImm(8);
//            BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::LW), Alex::T2).addReg(Alex::SP).addImm(12);
//            BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::LW), Alex::T2).addReg(Alex::SP).addImm(16);

            assert((MI->getOperand(0).getType() == MachineOperand::MachineOperandType::MO_Register) &&
                           "Call operation without a register");

            BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::CALL)).addReg(MI->getOperand(0).getReg());
            //BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::CALL)).addReg(MI->getOperand(2).getReg());
//            printf("call ");
//            for (auto i = 0; i < MI->getNumOperands(); ++i) {
//                printf("%d ", MI->getOperand(i).getType());
//            }
//            printf("\n");
        }

        // restore t0, t1, t2
        //BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::LW), Alex::T0).addReg(Alex::SP).addImm(0);
//        BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::LW), Alex::T1).addReg(Alex::SP).addImm(4);
//        BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::LW), Alex::T2).addReg(Alex::SP).addImm(8);
//        BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::LW), Alex::T3).addReg(Alex::SP).addImm(12);
//        BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::LW), Alex::T4).addReg(Alex::SP).addImm(16);
//        BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::ADDi), Alex::SP).addReg(Alex::SP).addImm(4*5);
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

unsigned AlexInstrInfo::GetInstSizeInBytes(const MachineInstr *MI) const {
    if (MI->getOpcode() ==  TargetOpcode::INLINEASM) {       // Inline Asm: Variable size.
        const MachineFunction *MF = MI->getParent()->getParent();
        const char *AsmStr = MI->getOperand(0).getSymbolName();
        return getInlineAsmLength(AsmStr, *MF->getTarget().getMCAsmInfo());
    }
}