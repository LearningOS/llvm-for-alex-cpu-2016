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
        :subtarget(subtarget), AlexGenInstrInfo(Alex::ADJCALLSTACKDOWN, Alex::ADJCALLSTACKUP) {

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

    switch (MI->getDesc().getOpcode()) {
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
            BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::LI), reg).addGlobalAddress(globalAddr, AlexII::MO_ABS_HI);
            BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::LIh), reg).addGlobalAddress(globalAddr, AlexII::MO_ABS_LO);
        }

        break;
    }

    case Alex::SEXT_INREG_1:
    case Alex::SEXT_INREG_8:
    case Alex::SEXT_INREG_16:
        lowerSExtPseudo(MI);
        break;

    case Alex::LHa:
    case Alex::LHs:
    case Alex::LBs:
    case Alex::LBa:
    case Alex::LBITa:
    case Alex::LBITs:
    case Alex::LBIT:
        lowerLoadExtendPseudo(MI);
        break;

    case Alex::CALLr:
    case Alex::CALLg:
        lowerCallPseudo(MI);
        break;

    case Alex::SELECT:
        lowerSelect(MI);
        break;
    case Alex::CombineLoHiRI: {
        auto Dest = MI->getOperand(0).getReg();
        auto Src = MI->getOperand(1).getReg();          // lo
        //auto GlobalAddress = MI->getOperand(2).getGlobal(); // hi
        BuildMI(*MI->getParent(), MI, MI->getDebugLoc(), get(Alex::ADDiu), Dest).addReg(Src).addImm(0);
        auto Operand2 = MI->getOperand(2);
        switch (Operand2.getType()) {
        case MachineOperand::MO_GlobalAddress:
            BuildMI(*MI->getParent(), MI, MI->getDebugLoc(), get(Alex::LIh), Dest)
                    .addGlobalAddress(Operand2.getGlobal(), 0, AlexII::MO_ABS_HI);
            break;
        case MachineOperand::MO_BlockAddress:
            BuildMI(*MI->getParent(), MI, MI->getDebugLoc(), get(Alex::LIh), Dest)
                    .addBlockAddress(Operand2.getBlockAddress(), 0, AlexII::MO_ABS_HI);
            break;
        case MachineOperand::MO_JumpTableIndex:
            BuildMI(*MI->getParent(), MI, MI->getDebugLoc(), get(Alex::LIh), Dest)
                    .addJumpTableIndex(Operand2.getIndex(), AlexII::MO_ABS_HI);
            break;
        default:
            llvm_unreachable("CombineLoHiRI unknown operand type");
        }

        break;
    }
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

bool AlexInstrInfo::lowerCallPseudo(MachineBasicBlock::iterator &MI) const {
    MachineBasicBlock &MBB = *MI->getParent();
    if (MI->getDesc().getOpcode() == Alex::CALLg) {
        // 这里不用备份T0, 由于T0用于返回值, 此处LLVM会标记T0为LiveIn
        BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::LIh), Alex::T0).addGlobalAddress(MI->getOperand(0).getGlobal(), AlexII::MO_ABS_HI);
        BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::ADDiu), Alex::T0).addReg(Alex::T0).addGlobalAddress(MI->getOperand(0).getGlobal(), AlexII::MO_ABS_LO);
        BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::CALL)).addReg(Alex::T0);
    }
    else {
        // temporary restore t0, t1, t2 in case that it uses t0~t2 as call addr
        auto regOperandIndex = MI->getNumOperands();
        for (int i = MI->getNumOperands() - 1; i >= 0; --i) {
            if (MI->getOperand(i).getType() == MachineOperand::MachineOperandType::MO_Register) {
                // assert(regOperandIndex == MI->getNumOperands() && "call operation with multiple registers!");
                regOperandIndex = static_cast<unsigned>(i);
            }
        }
        assert((regOperandIndex != MI->getNumOperands()) &&
               "Call operation without a register");
        BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::CALL)).addReg(MI->getOperand(regOperandIndex).getReg());
    }
    return true;
}
bool AlexInstrInfo::lowerLoadExtendPseudo(MachineBasicBlock::iterator &MI) const {
    MachineBasicBlock &MBB = *MI->getParent();
    switch (MI->getDesc().getOpcode()) {
    default:
        return false;
    case Alex::LHa:
    case Alex::LHs: {
        printf("lhs ");
        auto targetReg = MI->getOperand(0).getReg();
        auto srcReg = MI->getOperand(1).getReg();
        auto memOffset = MI->getOperand(2).getImm();
        BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::LH), targetReg).addReg(srcReg).addImm(memOffset);
        BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::SHL), targetReg).addReg(targetReg).addImm(16);
        BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::SAR), targetReg).addReg(targetReg).addImm(16);
        for (unsigned i = 0; i < MI->getNumOperands(); ++i) {
            printf("%d ", MI->getOperand(i).getType());
        }
        printf("\n");
        break;
    }
    case Alex::LBs:
    case Alex::LBa: {
        printf("lbs ");
        auto targetReg = MI->getOperand(0).getReg();
        auto srcReg = MI->getOperand(1).getReg();
        auto memOffset = MI->getOperand(2).getImm();
        BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::LB), targetReg).addReg(srcReg).addImm(memOffset);
        BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::SHL), targetReg).addReg(targetReg).addImm(24);
        BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::SAR), targetReg).addReg(targetReg).addImm(24);
        for (unsigned i = 0; i < MI->getNumOperands(); ++i) {
            printf("%d ", MI->getOperand(i).getType());
        }
        printf("\n");
        break;
    }
    case Alex::LBITa:
    case Alex::LBITs: {
        printf("lbits ");
        auto targetReg = MI->getOperand(0).getReg();
        auto srcReg = MI->getOperand(1).getReg();
        auto memOffset = MI->getOperand(2).getImm();
        BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::LB), targetReg).addReg(srcReg).addImm(memOffset);
        BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::SAR), targetReg).addReg(targetReg).addImm(31);
        for (unsigned i = 0; i < MI->getNumOperands(); ++i) {
            printf("%d ", MI->getOperand(i).getType());
        }
        printf("\n");
        break;
    }
    case Alex::LBIT: {
        printf("lhs ");
        auto targetReg = MI->getOperand(0).getReg();
        auto srcReg = MI->getOperand(1).getReg();
        auto memOffset = MI->getOperand(2).getImm();
        BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::LB), targetReg).addReg(srcReg).addImm(memOffset);
        BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::SHL), targetReg).addReg(targetReg).addImm(31);
        BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::SLR), targetReg).addReg(targetReg).addImm(31);
        for (unsigned i = 0; i < MI->getNumOperands(); ++i) {
            printf("%d ", MI->getOperand(i).getType());
        }
        printf("\n");
        break;
    }
    }
    return true;
}
bool AlexInstrInfo::lowerSExtPseudo(MachineBasicBlock::iterator &MI) const {
    MachineBasicBlock &MBB = *MI->getParent();
    switch(MI->getDesc().getOpcode()) {
    default:
        return false;
    case Alex::SEXT_INREG_1: {
        auto targetReg = MI->getOperand(0).getReg();
        auto srcReg = MI->getOperand(1).getReg();
        BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::ADDiu), targetReg).addReg(srcReg).addImm(0);
        BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::SHL), targetReg).addReg(targetReg).addImm(31);
        BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::SAR), targetReg).addReg(targetReg).addImm(31);
        printf("sext inreg 1 ");
        for (unsigned i = 0; i < MI->getNumOperands(); ++i) {
            printf("%d ", MI->getOperand(i).getType());
        }
        printf("\n");
        break;
    }
    case Alex::SEXT_INREG_8: {
        printf("sext inreg 8 ");
        auto targetReg = MI->getOperand(0).getReg();
        auto srcReg = MI->getOperand(1).getReg();
        BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::ADDiu), targetReg).addReg(srcReg).addImm(0);
        BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::SHL), targetReg).addReg(targetReg).addImm(24);
        BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::SAR), targetReg).addReg(targetReg).addImm(24);
        for (unsigned i = 0; i < MI->getNumOperands(); ++i) {
            printf("%d ", MI->getOperand(i).getType());
        }
        printf("\n");
    }
    case Alex::SEXT_INREG_16: {
        auto targetReg = MI->getOperand(0).getReg();
        auto srcReg = MI->getOperand(1).getReg();
        BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::ADDiu), targetReg).addReg(srcReg).addImm(0);
        BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::SHL), targetReg).addReg(targetReg).addImm(16);
        BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::SAR), targetReg).addReg(targetReg).addImm(16);
        printf("sext inreg 16");
        for (unsigned i = 0; i < MI->getNumOperands(); ++i) {
            printf("%d ", MI->getOperand(i).getType());
        }
        printf("\n");
        break;
    }
    }
    return true;
}

bool AlexInstrInfo::lowerSelect(MachineBasicBlock::iterator &MI) const {
    auto &MBB = *MI->getParent();
    auto targetReg =    MI->getOperand(0).getReg();
    auto conditionReg = MI->getOperand(1).getReg();
    auto trueReg =      MI->getOperand(2).getReg();
    auto falseReg =     MI->getOperand(3).getReg();

    // !a = (not a) and 1
    // targetReg = true * condition + false * !condition

    BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::MUL), targetReg).addReg(trueReg).addReg(conditionReg);

    // push true
    BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::ADDi), Alex::SP).addReg(Alex::SP).addImm(-4);
    BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::SW)).addReg(trueReg).addReg(Alex::SP).addImm(0);
    // condition = !condition
    BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::LI), trueReg).addImm(0xFFFF);
    BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::XOR), conditionReg).addReg(conditionReg).addReg(trueReg);
    BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::LI), trueReg).addImm(1);
    BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::AND), conditionReg).addReg(conditionReg).addReg(trueReg);

    // true * condition + false * !condition
    BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::MUL), trueReg).addReg(falseReg).addReg(conditionReg);
    BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::ADD), targetReg).addReg(targetReg).addReg(trueReg);

    // condition = !condition
    BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::LI), trueReg).addImm(0xFFFF);
    BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::XOR), conditionReg).addReg(conditionReg).addReg(trueReg);
    BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::LI), trueReg).addImm(1);
    BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::AND), conditionReg).addReg(conditionReg).addReg(trueReg);

    // pop true
    BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::LW), trueReg).addReg(Alex::SP).addImm(0);
    BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::ADDi), Alex::SP).addReg(Alex::SP).addImm(4);
    return true;
}


