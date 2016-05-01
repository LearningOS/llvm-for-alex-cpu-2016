#include <llvm/CodeGen/MachineMemOperand.h>
#include <MCTargetDesc/AlexBaseInfo.h>
#include <iostream>
#include "AlexInstrInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
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
    //unsigned ADDi = Alex::ADDi;
   // BuildMI(MBB, I, DL, get(ADDi), SP).addReg(SP).addImm(Amount);
    BuildMI(MBB, I, DL, get(Alex::V9ENT)).addImm(Amount);
}

MachineMemOperand *AlexInstrInfo::GetMemOperand(MachineBasicBlock &MBB, int FI,
                                                unsigned Flag) const {
    MachineFunction &MF = *MBB.getParent();
    MachineFrameInfo &MFI = *MF.getFrameInfo();
    unsigned Align = MFI.getObjectAlignment(FI);

    return MF.getMachineMemOperand(MachinePointerInfo::getFixedStack(MF, FI, 0), Flag,
                                   static_cast<uint64_t>(MFI.getObjectSize(FI)), Align);
}

void AlexInstrInfo::expandRetLR(MachineBasicBlock &MBB,
                                  MachineBasicBlock::iterator I) const {
    //BuildMI(MBB, I, I->getDebugLoc(), get(Alex::RET));
    BuildMI(MBB, I, I->getDebugLoc(), get(Alex::V9LEV));
}

bool AlexInstrInfo::expandPostRAPseudo(MachineBasicBlock::iterator MI) const {
    if (MI->getDesc().getOpcode() == Alex::V9BLE_PS) {
        printf("hahaaha-----------------\n");
    }

//@expandPostRAPseudo-body
    MachineBasicBlock &MBB = *MI->getParent();
    bool ret = true;
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
            //BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::LI), reg).addImm(val & 0xFFFF);
            //BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::LIh), reg).addImm((val >> 16) & 0xFFFF);
            lowerLI24(MI, reg, static_cast<uint16_t>(val & 0xFFFF));
            lowerLIH8(MI, reg, static_cast<uint16_t>((val >> 16) & 0xFFFF));

            printf("li32 imm: %d\n", (int)val);
        }
        else {
            auto globalAddr = immOprand.getGlobal();
            //BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::LI), reg).addGlobalAddress(globalAddr, 0, AlexII::MO_ABS_HI);
            //BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::LIh), reg).addGlobalAddress(globalAddr, 0, AlexII::MO_ABS_LO);
            lowerLI32(MI, reg, globalAddr);
        }

        break;
    }

    case Alex::SEXT_INREG_1:
    case Alex::SEXT_INREG_8:
    case Alex::SEXT_INREG_16:
        ret = lowerSExtPseudo(MI);
        break;

    case Alex::LHa:
    case Alex::LHs:
    case Alex::LBs:
    case Alex::LBa:
    case Alex::LBITa:
    case Alex::LBITs:
    case Alex::LBIT:
        ret = lowerLoadExtendPseudo(MI);
        break;

    case Alex::CALLr:
    case Alex::CALLg:
        ret = lowerCallPseudo(MI);
        break;

    case Alex::SELECT:
        ret = lowerSelect(MI);
        break;

    case Alex::ADD:
    case Alex::SUB:
    case Alex::MUL:
    case Alex::DIV:
        ret = lowerArithLogicRRR(MI, MI->getDesc().getOpcode());
        printf("lower arith-logic opcode %d -----------------------------\n", MI->getDesc().getOpcode());
        break;

    case Alex::V9ADDi_PS: {
        auto resultReg = MI->getOperand(0).getReg();
        auto srcReg = MI->getOperand(1).getReg();
        auto Operand2 = MI->getOperand(2);
        if (Operand2.getType() == MachineOperand::MO_GlobalAddress && MI->getDesc().getOpcode() == Alex::V9ADDi_PS) {
//            lowerPushR(MI, Alex::S0);
//            lowerPushR(MI, Alex::S1);
//            lowerMov(MI, Alex::S0, srcReg);
//           // lowerPushI(MI, Imm);
//            BuildMI(*MI->getParent(), MI, MI->getDebugLoc(), get(Alex::V9PUSHI)).addGlobalAddress(Operand2.getGlobal(), 0, AlexII::MO_ABS_LO);
//            lowerPopR(MI, Alex::S1);
//            BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::V9ADD));
//            lowerMov(MI, resultReg, Alex::S0);
//            lowerPopR(MI, Alex::S1);
//            lowerPopR(MI, Alex::S0);
//            printf("addi global address\n");
            llvm_unreachable("lower addi with global address");

        }
        else if (Operand2.getType() == MachineOperand::MO_Immediate) {
            auto imm24 = static_cast<uint32_t>(Operand2.getImm() & 0xFFFFFF);
            lowerArithLogicRRI(MI, MI->getDesc().getOpcode(), resultReg, srcReg, imm24);
        }
        break;
    }

    case Alex::V9LT_PS:
    case Alex::V9GT_PS:
    case Alex::V9LE_PS:
    case Alex::V9GE_PS:
    case Alex::V9EQ_PS:
    case Alex::V9NE_PS:
        ret = lowerV9Cmp(MI, MI->getDesc().getOpcode());
        printf("lower v9 comparason, opcode %d\n", MI->getDesc().getOpcode());
        break;

    case Alex::V9BLT_PS:
    case Alex::V9BGT_PS:
    case Alex::V9BLE_PS:
    case Alex::V9BGE_PS:
    case Alex::V9BEQ_PS:
    case Alex::V9BNE_PS:
        ret = lowerV9Branch(MI, MI->getDesc().getOpcode());
        break;
    case Alex::V9LW_PS: {
        unsigned Dest = MI->getOperand(0).getReg();
        unsigned Base = MI->getOperand(1).getReg();
        if (Base == Alex::SP) {
            auto FI = static_cast<int32_t>(MI->getOperand(2).getImm());
            ret = lowerLoadFI(MI, FI, Dest);
        }
        else {
            auto Operand2 = MI->getOperand(2);
            if (Operand2.getType() == MachineOperand::MO_GlobalAddress) {
                //ret = lowerLoadMem(MI, Dest, Base, Operand2.getGlobal());
            }
            else {
                auto Offset = static_cast<int32_t>(MI->getOperand(2).getImm());
                ret = lowerLoadMem(MI, Dest, Base, Offset);
            }
        }

        break;
    }
    case Alex::V9SW_PS: {
        unsigned Src = MI->getOperand(0).getReg();
        unsigned Base = MI->getOperand(1).getReg();
        if (Base == Alex::SP) {
            auto FI = static_cast<int32_t>(MI->getOperand(2).getImm());
            lowerStoreFI(MI, FI, Src);
        }
        else {
            auto Offset = static_cast<int32_t>(MI->getOperand(2).getImm());
            ret = lowerStoreMem(MI, Src, Base, Offset);
        }

        break;
    }
    case Alex::PUSHr: {
        auto Reg = MI->getOperand(0).getReg();
        lowerPushR(MI, Reg);
        break;
    }
    case Alex::POPr: {
        auto Reg = MI->getOperand(0).getReg();
        lowerPopR(MI, Reg);
        break;
    }
    case Alex::V9LI24_PS: {
        auto Reg = MI->getOperand(0).getReg();
        auto Operand1 = MI->getOperand(1);
        if (Operand1.getType() == MachineOperand::MO_Immediate) {
            lowerLI24(MI, Reg, static_cast<uint32_t>(Operand1.getImm()));
            printf("lowerli24 imm: %d\n", (int)Operand1.getImm());
        }
        else if(Operand1.getType() == MachineOperand::MO_GlobalAddress) {
            lowerLI24(MI, Reg, Operand1.getGlobal());
        }
        else {
            llvm_unreachable("li24 unknown operand");
        }

        break;
    }
    case Alex::V9LIh8_PS: {
        auto Reg = MI->getOperand(0).getReg();
        auto Operand1 = MI->getOperand(1);
        if (Operand1.getType() == MachineOperand::MO_Immediate) {
            lowerLIH8(MI, Reg, static_cast<uint16_t>(Operand1.getImm() >> 16));
        }
        else if(Operand1.getType() == MachineOperand::MO_GlobalAddress) {
            lowerLIH8(MI, Reg, Operand1.getGlobal());
        }
        else {
            llvm_unreachable("lih8 unknown operand");
        }
        break;
    }
    case Alex::V9LI32_PS: {
        auto Reg = MI->getOperand(0).getReg();
        auto Operand1 = MI->getOperand(1);
        if (Operand1.getType() == MachineOperand::MO_Immediate) {
            lowerLI32(MI, Reg, static_cast<uint32_t>(Operand1.getImm()));
            printf("lowerli32 imm: %d\n", (int)Operand1.getImm());
        }
        else if(Operand1.getType() == MachineOperand::MO_GlobalAddress) {
            lowerLI32(MI, Reg, Operand1.getGlobal());
        }
        else {
            llvm_unreachable("li32 unknown operand");
        }
        break;
    }

    }

    MBB.erase(MI);
    return ret;
}

void AlexInstrInfo::storeRegToStackSlot(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
                unsigned SrcReg, bool isKill, int FI,
                const TargetRegisterClass *, const TargetRegisterInfo *) const {
    DebugLoc DL;
    if (I != MBB.end()) DL = I->getDebugLoc();
//    MachineMemOperand *MMO = GetMemOperand(MBB, FI, MachineMemOperand::MOStore);
//
//    unsigned Opc = 0;
//
//    Opc = Alex::SW;
//    assert(Opc && "Register class not handled!");
//    BuildMI(MBB, I, DL, get(Opc)).addReg(SrcReg, getKillRegState(isKill))
    //        .addFrameIndex(FI).addImm(0).addMemOperand(MMO);
    //SrcReg = Reg.getReg();
//    if (SrcReg == 0x80000005) {
//        SrcReg = Alex::S0;
//        printf("store reg to stack slot 0x80000005\n");
//    }
//
//    if (SrcReg == Alex::S0) {
//        BuildMI(MBB, I, DL, get(Alex::V9SL)).addImm(FI).addReg(Alex::S0, getKillRegState(isKill));
//    }
//    else {
//        lowerPush(I, Alex::S0);
//        lowerMov(I, Alex::S0, SrcReg);
//        // load
//        BuildMI(MBB, I, DL, get(Alex::V9SL)).addImm(FI).addReg(Alex::S0, getKillRegState(isKill));
//        lowerPop(I, Alex::S0);
//    }
    MachineInstr &MI = *I;
    MachineFunction &MF = *MI.getParent()->getParent();
    MachineFrameInfo *MFI = MF.getFrameInfo();

    unsigned i = 0;
    while (!MI.getOperand(i).isFI()) {
        ++i;
        assert(i < MI.getNumOperands() &&
               "Instr doesn't have FrameIndex operand!");
    }


    int FrameIndex = MI.getOperand(i).getIndex();
    uint64_t stackSize = MF.getFrameInfo()->getStackSize();
    int64_t spOffset = MF.getFrameInfo()->getObjectOffset(FrameIndex);

    const std::vector<CalleeSavedInfo> &CSI = MFI->getCalleeSavedInfo();
    int MinCSFI = 0;
    int MaxCSFI = -1;

    if (CSI.size()) {
        MinCSFI = CSI[0].getFrameIdx();
        MaxCSFI = CSI[CSI.size() - 1].getFrameIdx();
    }

    int64_t Offset;
    Offset = spOffset + stackSize;


    lowerStoreFI(I, FI, SrcReg);
}


void AlexInstrInfo::loadRegFromStackSlot(MachineBasicBlock &MBB, MachineBasicBlock::iterator MI,
                 unsigned DestReg, int FI, const TargetRegisterClass *,
                 const TargetRegisterInfo *) const {
    DebugLoc DL;
    if (MI != MBB.end()) DL = MI->getDebugLoc();

//    if (DestReg == Alex::S0) {
//        BuildMI(MBB, MI, DL, get(Alex::V9LL)).addImm(FI);
//    }
//    else {
//        // push RA
//        lowerPush(MI, Alex::S0);
//        // load
//        BuildMI(MBB, MI, DL, get(Alex::V9LL)).addImm(FI);
//        lowerMov(MI, DestReg, Alex::S0);
//        lowerPop(MI, Alex::S0);
//    }
    lowerLoadFI(MI, FI, DestReg);

}

void AlexInstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                                  MachineBasicBlock::iterator I, DebugLoc DL,
                                  unsigned DestReg, unsigned SrcReg,
                                  bool KillSrc) const {
//    unsigned Opc = 0;
//
//    if (Alex::Int32RegsRegClass.contains(DestReg)) { // Copy to CPU Reg.
//        if (Alex::Int32RegsRegClass.contains(SrcReg))
//            Opc = Alex::ADDi;
//    }
//
//    assert(Opc  && DestReg && SrcReg && "Cannot copy registers");
//
//    BuildMI(MBB, I, DL, get(Opc), DestReg).
//                    addReg(SrcReg, getKillRegState(KillSrc)).
//                    addImm(0);
    lowerMov(I, DestReg, SrcReg);
}

bool AlexInstrInfo::lowerCallPseudo(MachineBasicBlock::iterator &MI) const {
    MachineBasicBlock &MBB = *MI->getParent();
    unsigned FuncAddrReg = 0;
    if (MI->getDesc().getOpcode() == Alex::CALLg) {
        // 这里不用备份T0, 由于T0用于返回地址, 此处LLVM会标记T0为LiveIn
        FuncAddrReg = Alex::S0;
        BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::LIh), FuncAddrReg).addGlobalAddress(MI->getOperand(0).getGlobal(), 0, AlexII::MO_ABS_HI);
        //BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::ADDiu), FuncAddrReg).addReg(FuncAddrReg).addGlobalAddress(MI->getOperand(0).getGlobal(), 0, AlexII::MO_ABS_LO);
    }
    else {
        // temporary restore t0, t1, t2 in case that it uses t0~t2 as call addr
        auto regOperandIndex = MI->getNumOperands();
        for (unsigned i = 0; i < MI->getNumOperands(); ++i) {
            if (MI->getOperand(i).getType() == MachineOperand::MachineOperandType::MO_Register) {
                // assert(regOperandIndex == MI->getNumOperands() && "call operation with multiple registers!");
                regOperandIndex = i;
            }
        }
        assert((regOperandIndex != MI->getNumOperands()) &&
               "Call operation without a register");
        FuncAddrReg = MI->getOperand(regOperandIndex).getReg();
    }
    //BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::CALL)).addReg(FuncAddrReg);
    lowerAbsoluteCall(MI, FuncAddrReg);
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
    }
    case Alex::LBITa:
    case Alex::LBITs: {
        printf("lbits ");
        auto targetReg = MI->getOperand(0).getReg();
        auto srcReg = MI->getOperand(1).getReg();
        auto memOffset = MI->getOperand(2).getImm();
        BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::LB), targetReg).addReg(srcReg).addImm(memOffset);
        BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::SHL), targetReg).addReg(targetReg).addImm(31);
        BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::SAR), targetReg).addReg(targetReg).addImm(31);
        for (unsigned i = 0; i < MI->getNumOperands(); ++i) {
            printf("%d ", MI->getOperand(i).getType());
        }
        printf("\n");
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
        //BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::ADDiu), targetReg).addReg(srcReg).addImm(0);
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
        //BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::ADDiu), targetReg).addReg(srcReg).addImm(0);
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
        //BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::ADDiu), targetReg).addReg(srcReg).addImm(0);
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
    //BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::ADDi), Alex::SP).addReg(Alex::SP).addImm(-4);
    //BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::SW)).addReg(trueReg).addReg(Alex::SP).addImm(0);
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
    //BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::LW), trueReg).addReg(Alex::SP).addImm(0);
    //BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::ADDi), Alex::SP).addReg(Alex::SP).addImm(4);
    return true;
}

bool AlexInstrInfo::lowerArithLogicRRR(MachineBasicBlock::iterator &MI, unsigned Opcode) const {
    auto resultReg = MI->getOperand(0).getReg();
    auto lhsReg = MI->getOperand(1).getReg();
    auto rhsReg = MI->getOperand(2).getReg();
    lowerArithLogicRRR(MI, Opcode, resultReg, lhsReg, rhsReg);
    return true;
}

bool AlexInstrInfo::lowerArithLogicRRR(MachineBasicBlock::iterator &MI, unsigned Opcode, unsigned resultReg,
                                       unsigned lhsReg, unsigned rhsReg) const {
    auto &MBB = *MI->getParent();

    if (!(resultReg == Alex::S0 && lhsReg == Alex::S0 && rhsReg == Alex::S1)) {
        lowerPushR(MI, Alex::S0);
        lowerPushR(MI, Alex::S1);
        lowerPushR(MI, lhsReg);
        lowerPushR(MI, rhsReg);
        lowerPopR(MI, Alex::S0);
        lowerPopR(MI, Alex::S1);
    }

    switch(Opcode) {
    default:
        return false;
    case Alex::ADD:
        BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::V9ADD));
        break;
    case Alex::SUB:
        BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::V9SUB));
        break;
    case Alex::MUL:
        BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::V9MUL));
        break;
    case Alex::DIV:
        BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::V9DIV));
        break;
    }

    if (!(resultReg == Alex::S0 && lhsReg == Alex::S0 && rhsReg == Alex::S1)) {
        lowerMov(MI, resultReg, Alex::S0);
        lowerPopR(MI, Alex::S1);
        lowerPopR(MI, Alex::S0);
    }

    return true;
}

bool AlexInstrInfo::lowerArithLogicRRI(MachineBasicBlock::iterator &MI, unsigned Opcode,
                                       unsigned resultReg, unsigned srcReg, unsigned Imm) const {
    auto &MBB = *MI->getParent();
    auto Imm24 = Imm & 0xFFFFFF;
    unsigned alexOpc;
    switch(Opcode) {
    default:
        return false;
    case Alex::V9ADDi_PS:
        alexOpc = Alex::V9ADDi;
        break;
    //case Alex::V9SUBi_PS:
    //    alexOpc = Alex::V9SUBi;
    //    break;
    //case Alex::V9MULi_PS:
    //    alexOpc = Alex::V9MULi;
    //    break;
    //case Alex::V9DIVi_PS:
    //    alexOpc = Alex::V9DIVi;
    //    break;
    }

    if (resultReg == Alex::S0 && srcReg == Alex::S0) {
        BuildMI(MBB, MI, MI->getDebugLoc(), get(alexOpc)).addImm(Imm24);
    }
    else if (resultReg == Alex::S0 /*&& srcReg != Alex::S0*/) {
        lowerMov(MI, Alex::S0, srcReg);
        BuildMI(MBB, MI, MI->getDebugLoc(), get(Opcode)).addImm(Imm24);
    }
    else if(/*resultReg != Alex::S0 &&*/ srcReg != Alex::S0) {
        lowerPushR(MI, Alex::S0);
        lowerMov(MI, Alex::S0, srcReg);

        lowerMov(MI, Alex::S0, srcReg);
        BuildMI(MBB, MI, MI->getDebugLoc(), get(Opcode)).addImm(Imm24);

        lowerMov(MI, resultReg, Alex::S0);
        lowerPopR(MI, Alex::S0);
    }
    else /* resultReg != Alex::S0 && srcReg == Alex::S0 */ {
        lowerPush(MI, Alex::S0);

        BuildMI(MBB, MI, MI->getDebugLoc(), get(Opcode)).addImm(Imm24);

        lowerMov(MI, resultReg, Alex::S0);
        lowerPopR(MI, Alex::S0);
    }

    return true;
}

bool AlexInstrInfo::lowerPush(MachineBasicBlock::iterator &MI, unsigned Reg) const {
    lowerPushR(MI, Reg);
    //BuildMI(*MI->getParent(), MI, MI->getDebugLoc(), get(Alex::PUSHr)).addReg(Reg);
    return true;
}

bool AlexInstrInfo::lowerPop(MachineBasicBlock::iterator &MI, unsigned Reg) const {
    //BuildMI(*MI->getParent(), MI, MI->getDebugLoc(), get(Alex::POPr), Reg);
    lowerPopR(MI, Reg);
    return true;
}

bool AlexInstrInfo::lowerV9Cmp(MachineBasicBlock::iterator &MI, unsigned Opcode) const {
    auto &MBB = *MI->getParent();
    auto targetReg = MI->getOperand(0).getReg();
    auto lhsReg = MI->getOperand(1).getReg();
    auto rhsReg = MI->getOperand(2).getReg();


    // push RB
    lowerPush(MI, Alex::S1);
    // push $ra
    lowerPush(MI, lhsReg);
    // push $rb
    lowerPush(MI, rhsReg);

    // pop RA
    lowerPop(MI, Alex::S0);
    // pop RB
    lowerPop(MI, Alex::S1);
    unsigned V9Opcode = 0;
    switch(Opcode) {
    default:
        return false;
    case Alex::V9LT_PS:
        V9Opcode = Alex::V9LT;
        break;
    case Alex::V9GT_PS:
        V9Opcode = Alex::V9GT;
        break;
    case Alex::V9LE_PS:
        V9Opcode = Alex::V9LE;
        break;
    case Alex::V9GE_PS:
        V9Opcode = Alex::V9GE;
        break;
    case Alex::V9EQ_PS:
        V9Opcode = Alex::V9EQ;
        break;
    case Alex::V9NE_PS:
        V9Opcode = Alex::V9NE;
        break;
    }
    BuildMI(MBB, MI, MI->getDebugLoc(), get(V9Opcode));
    // pop RB
    lowerPop(MI, Alex::S1);

    lowerPush(MI, Alex::S0);
    lowerPop(MI, targetReg);

    return true;
}

bool AlexInstrInfo::lowerV9Branch(MachineBasicBlock::iterator &MI, unsigned int Opcode) const {
    auto &MBB = *MI->getParent();
    auto lhsReg = MI->getOperand(0).getReg();
    auto rhsReg = MI->getOperand(1).getReg();
    auto targetBlock = MI->getOperand(2).getMBB();

    // push RB
    lowerPush(MI, Alex::S1);
    // push $ra
    lowerPush(MI, lhsReg);
    // push $rb
    lowerPush(MI, rhsReg);

    // pop RA
    lowerPop(MI, Alex::S1);
    // pop RB
    lowerPop(MI, Alex::S0);
    // RA = $lhs ?? $rhs

    unsigned V9Opcode = 0;
    switch(Opcode) {
    default:
        return false;
    case Alex::V9BLT_PS:
        V9Opcode = Alex::V9LT;
        break;
    case Alex::V9BGT_PS:
        V9Opcode = Alex::V9GT;
        break;
    case Alex::V9BLE_PS:
        V9Opcode = Alex::V9LE;
        break;
    case Alex::V9BGE_PS:
        V9Opcode = Alex::V9GE;
        break;
    case Alex::V9BEQ_PS:
        V9Opcode = Alex::V9EQ;
        break;
    case Alex::V9BNE_PS:
        V9Opcode = Alex::V9NE;
        break;
    }
    BuildMI(MBB, MI, MI->getDebugLoc(), get(V9Opcode));
    // pop RB
    lowerPop(MI, Alex::S1);
    BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::V9BNEZ)).addMBB(targetBlock);
    printf("lower v9 branch, opcode %d\n", MI->getDesc().getOpcode());
    return true;
}

bool AlexInstrInfo::lowerMov(MachineBasicBlock::iterator &MI, unsigned Dest, unsigned Src) const {

    if (Dest == Alex::S1 && Src == Alex::S0) {
        BuildMI(*MI->getParent(), MI, MI->getDebugLoc(), get(Alex::V9LBA));
    }
    else if (Dest == Alex::S2 && Src == Alex::S0) {
        BuildMI(*MI->getParent(), MI, MI->getDebugLoc(), get(Alex::V9LCA));
    }
    else if (Dest != Src) {
        lowerPush(MI, Src);
        lowerPop(MI, Dest);
    }
    return true;
}

bool AlexInstrInfo::lowerAbsoluteCall(MachineBasicBlock::iterator &MI, unsigned Reg) const {
    auto &MBB = *MI->getParent();

    // push retaddr
    // push reg
    // lev
    if (Alex::S0 == Reg) {
        BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::V9ENT)).addImm(-8*2);
        lowerStoreFI(MI, 0, Alex::S0);

        BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::LEAg)).addImm(4*3);
        BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::V9SL)).addImm(8);
        BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::V9LL)).addImm(0);
    }
    else {
        BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::V9ENT)).addImm(-8*3);
        lowerStoreFI(MI, 0, Alex::S0);
        lowerStoreFI(MI, 8, Reg);

        BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::LEAg)).addImm(4*3);
        // S0 = SP[12]
        BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::V9SL)).addImm(12);
        lowerPopR(MI, Alex::S0);
    }
    BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::V9LEV));
    return true;
}

bool AlexInstrInfo::lowerLoadFI(MachineBasicBlock::iterator &MI, int32_t FI, unsigned Reg) const {
    auto &MBB = *MI->getParent();

    DebugLoc DL;
    if (MI != MBB.end()) DL = MI->getDebugLoc();

    if (Reg == Alex::S0) {
        BuildMI(MBB, MI, DL, get(Alex::V9LL)).addImm(FI);
    }
    else if (Reg == Alex::S1) {
        BuildMI(MBB, MI, DL, get(Alex::V9LBL)).addImm(FI);
    }
    else if (Reg == Alex::S2) {
        BuildMI(MBB, MI, DL, get(Alex::V9LCL)).addImm(FI);
    }
    else {
        lowerPush(MI, Alex::S0);
        BuildMI(MBB, MI, DL, get(Alex::V9LL)).addImm(FI+8); // frame alignment should be 4
        lowerPush(MI, Alex::S0);
        lowerPop(MI, Reg);
        lowerPop(MI, Alex::S0);
    }

    return true;
}


bool AlexInstrInfo::lowerLoadMem(MachineBasicBlock::iterator &MI, unsigned Dst, unsigned Base, int Offset) const {
    auto &MBB = *MI->getParent();
    if (Dst == Alex::S0 && Base == Alex::S0) {
        lowerPushR(MI, Alex::S1);

        lowerLI32(MI, Alex::S1, static_cast<uint32_t>(Offset));
        BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::V9ADD));
        BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::V9LX)).addImm(0);

        lowerPopR(MI, Alex::S1);
    }
    else if (Dst == Alex::S0 && Base != Alex::S0 && Base != Alex::S1) {
        lowerPushR(MI, Alex::S1);

        lowerLI32(MI, Alex::S0, static_cast<uint32_t>(Offset));
        lowerMov(MI, Alex::S1, Base);
        BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::V9ADD));

        lowerPopR(MI, Alex::S1);

        BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::V9LX)).addImm(0);
    }
    else if (Dst == Alex::S0 && Base == Alex::S1) {
        lowerLI32(MI, Alex::S0, static_cast<uint32_t>(Offset));
        BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::V9ADD));
        BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::V9LX)).addImm(0);
    }
    else /* Dst != Alex::S0 */ {
        lowerPushR(MI, Alex::S0);
        lowerPushR(MI, Alex::S1);
        lowerPushR(MI, Base);

        lowerMov(MI, Alex::S0, Base);
        lowerLI32(MI, Alex::S1, Offset);
        BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::V9ADD));
        BuildMI(MBB, MI, MI->getDebugLoc(), get(Alex::V9LX)).addImm(0);

        lowerPopR(MI, Base);
        lowerPopR(MI, Alex::S1);
        lowerPopR(MI, Alex::S0);
    }
    return true;
}


bool AlexInstrInfo::lowerStoreFI(MachineBasicBlock::iterator &MI, int32_t FI, unsigned int Reg) const {
    auto &MBB = *MI->getParent();

    //if (FI % 8 != 0) {
    //    llvm_unreachable("fi unaligned");
    //}
    printf("lower store fi %d\n", FI);

    DebugLoc DL;
    if (MI != MBB.end()) DL = MI->getDebugLoc();
    if (Reg == Alex::S0) {
        BuildMI(MBB, MI, DL, get(Alex::V9SL)).addImm(FI);
    }
    else {
        lowerPush(MI, Alex::S0);
        lowerMov(MI, Alex::S0, Reg);
        BuildMI(MBB, MI, DL, get(Alex::V9SL)).addImm(FI+8); // frame alignment 4
        lowerPop(MI, Alex::S0);
    }

    return true;
}

bool AlexInstrInfo::lowerPushR(MachineBasicBlock::iterator &MI, unsigned Reg) const {
    auto &MBB = *MI->getParent();
    unsigned Opcode = 0;
    switch(Reg) {
    default:
        printf("unknown register: %d\n", Reg);
        llvm_unreachable("lowerPushR unknown register");
    case Alex::S0:
        Opcode = Alex::V9PUSHA;
        break;
    case Alex::S1:
        Opcode = Alex::V9PUSHB;
        break;
    case Alex::S2:
        Opcode = Alex::V9PUSHC;
        break;
    case Alex::T0:
        Opcode = Alex::V9PUSHD;
        break;
    }
    BuildMI(MBB, MI, MI->getDebugLoc(), get(Opcode));
    return true;
}

bool AlexInstrInfo::lowerPopR(MachineBasicBlock::iterator &MI, unsigned Reg) const {
    unsigned Opcode = 0;
    switch(Reg) {
    default:
        llvm_unreachable("lowerPushR unknown register");
    case Alex::S0:
        Opcode = Alex::V9POPA;
        break;
    case Alex::S1:
        Opcode = Alex::V9POPB;
        break;
    case Alex::S2:
        Opcode = Alex::V9POPC;
        break;
    case Alex::T0:
        Opcode = Alex::V9POPD;
        break;
    }
    BuildMI(*MI->getParent(), MI, MI->getDebugLoc(), get(Opcode));
    return true;
}

bool AlexInstrInfo::lowerPushI(MachineBasicBlock::iterator &MI, unsigned Imm16, unsigned Flags) const {
    BuildMI(*MI->getParent(), MI, MI->getDebugLoc(), get(Alex::V9PUSHI)).addImm(Imm16 & 0xFFFFFF);
    return true;
}

bool AlexInstrInfo::lowerLI24(MachineBasicBlock::iterator &MI, unsigned Reg, uint32_t Imm) const {
    if (Reg == Alex::S0) {
        BuildMI(*MI->getParent(), MI, MI->getDebugLoc(), get(Alex::V9LI)).addImm(Imm & 0xFFFFFF);
    }
    else {
        BuildMI(*MI->getParent(), MI, MI->getDebugLoc(), get(Alex::V9PUSHI)).addImm(Imm & 0xFFFFFF);
        lowerPopR(MI, Reg);
    }
    printf("v9 li24 pseudo: li %d\n", Imm);
    return true;
}

bool AlexInstrInfo::lowerLI24(MachineBasicBlock::iterator &MI, unsigned Reg, const GlobalValue *GlobalAddress) const {
    if (Reg == Alex::S0) {
        BuildMI(*MI->getParent(), MI, MI->getDebugLoc(), get(Alex::V9LI)).addGlobalAddress(
                GlobalAddress, 0, AlexII::MO_ABS_LO);
    }
    else {
        BuildMI(*MI->getParent(), MI, MI->getDebugLoc(), get(Alex::V9PUSHI)).addGlobalAddress(
                GlobalAddress, 0, AlexII::MO_ABS_LO);
        lowerPopR(MI, Reg);
    }
    printf("v9 li24 pseudo: li global address\n");
    return true;
}

bool AlexInstrInfo::lowerLIH8(MachineBasicBlock::iterator &MI, unsigned Reg, uint32_t Imm) const {
    if (Reg == Alex::S0) {
        // reg = reg + Imm << 24
        lowerPushR(MI, Alex::S0);

        // b = Imm & 0xFF000000
        BuildMI(*MI->getParent(), MI, MI->getDebugLoc(), get(Alex::V9LI)).addImm((Imm >> 24) & 0xFF);
        BuildMI(*MI->getParent(), MI, MI->getDebugLoc(), get(Alex::V9LIh)).addImm(0);
        BuildMI(*MI->getParent(), MI, MI->getDebugLoc(), get(Alex::V9LBA));

        lowerPopR(MI, Alex::S0);
        BuildMI(*MI->getParent(), MI, MI->getDebugLoc(), get(Alex::V9ADD));
    }
    else {
        lowerPushR(MI, Alex::S0);
        lowerMov(MI, Alex::S0, Reg);

        lowerPushR(MI, Alex::S0);
        BuildMI(*MI->getParent(), MI, MI->getDebugLoc(), get(Alex::V9LI)).addImm((Imm >> 24) & 0xFF);
        BuildMI(*MI->getParent(), MI, MI->getDebugLoc(), get(Alex::V9LIh)).addImm(0);
        BuildMI(*MI->getParent(), MI, MI->getDebugLoc(), get(Alex::V9LBA));
        lowerPopR(MI, Alex::S0);
        BuildMI(*MI->getParent(), MI, MI->getDebugLoc(), get(Alex::V9ADD));

        lowerMov(MI, Reg, Alex::S0);
        lowerPopR(MI, Alex::S0);
    }

    printf("v9 lih8 pseudo: lih %d\n", (int16_t)Imm);
    return true;
}

bool AlexInstrInfo::lowerLIH8(MachineBasicBlock::iterator &MI, unsigned Reg, const GlobalValue *GlobalAddress) const {
    if (Reg == Alex::S0) {
        // reg = reg + Imm << 24
        lowerPushR(MI, Alex::S0);
        BuildMI(*MI->getParent(), MI, MI->getDebugLoc(), get(Alex::V9LI)).addGlobalAddress(GlobalAddress, 0, AlexII::MO_ABS_HI);
        BuildMI(*MI->getParent(), MI, MI->getDebugLoc(), get(Alex::V9LIh)).addImm(0);
        BuildMI(*MI->getParent(), MI, MI->getDebugLoc(), get(Alex::V9LBA));
        lowerPopR(MI, Alex::S0);
        BuildMI(*MI->getParent(), MI, MI->getDebugLoc(), get(Alex::V9ADD));
    }
    else {
        lowerPushR(MI, Alex::S0);
        lowerMov(MI, Alex::S0, Reg);

        lowerPushR(MI, Alex::S0);
        BuildMI(*MI->getParent(), MI, MI->getDebugLoc(), get(Alex::V9LI)).addGlobalAddress(GlobalAddress, 0, AlexII::MO_ABS_HI);
        BuildMI(*MI->getParent(), MI, MI->getDebugLoc(), get(Alex::V9LIh)).addImm(0);
        BuildMI(*MI->getParent(), MI, MI->getDebugLoc(), get(Alex::V9LBA));
        lowerPopR(MI, Alex::S0);
        BuildMI(*MI->getParent(), MI, MI->getDebugLoc(), get(Alex::V9ADD));

        lowerMov(MI, Reg, Alex::S0);
        lowerPopR(MI, Alex::S0);
    }
    printf("v9 lih9 pseudo: lih global address\n");
    return true;
}

bool AlexInstrInfo::lowerLI32(MachineBasicBlock::iterator &MI, unsigned Reg, uint32_t Imm) const {

    if (Reg == Alex::S0) {
        //lowerLI24(MI, Reg, static_cast<uint16_t>(Imm & 0xFFFFFF));
        //lowerLIH8(MI, Alex::S0, static_cast<uint16_t>((Imm >> 24) & 0xFF));
        lowerLI24(MI, Reg, static_cast<uint16_t>((Imm>>24) & 0xFF));
        BuildMI(*MI->getParent(), MI, MI->getDebugLoc(), get(Alex::V9LIh)).addImm(0);
        BuildMI(*MI->getParent(), MI, MI->getDebugLoc(), get(Alex::V9ADDi)).addImm(Imm & 0xFFFFFF);
    }
    else {
        lowerPush(MI, Alex::S0);
        lowerLI24(MI, Reg, static_cast<uint16_t>((Imm>>24) & 0xFF));
        BuildMI(*MI->getParent(), MI, MI->getDebugLoc(), get(Alex::V9LIh)).addImm(0);
        BuildMI(*MI->getParent(), MI, MI->getDebugLoc(), get(Alex::V9ADDi)).addImm(Imm & 0xFFFFFF);
        lowerMov(MI, Reg, Alex::S0);
        lowerPopR(MI, Alex::S0);
    }

    return true;
}

bool AlexInstrInfo::lowerLI32(MachineBasicBlock::iterator &MI, unsigned Reg, const GlobalValue *GlobalAddr) const {

    if (Reg == Alex::S0) {
        BuildMI(*MI->getParent(), MI, MI->getDebugLoc(), get(Alex::V9LI)).addGlobalAddress(
                GlobalAddr, 0, AlexII::MO_ABS_HI);
        BuildMI(*MI->getParent(), MI, MI->getDebugLoc(), get(Alex::V9LIh)).addImm(0);
        BuildMI(*MI->getParent(), MI, MI->getDebugLoc(), get(Alex::V9ADDi)).addGlobalAddress(GlobalAddr, AlexII::MO_ABS_LO);
    }
    else {
        lowerPush(MI, Alex::S0);
        BuildMI(*MI->getParent(), MI, MI->getDebugLoc(), get(Alex::V9LI)).addGlobalAddress(
                GlobalAddr, 0, AlexII::MO_ABS_HI);
        BuildMI(*MI->getParent(), MI, MI->getDebugLoc(), get(Alex::V9LIh)).addImm(0);
        BuildMI(*MI->getParent(), MI, MI->getDebugLoc(), get(Alex::V9ADDi)).addGlobalAddress(GlobalAddr, AlexII::MO_ABS_LO);
        lowerMov(MI, Reg, Alex::S0);
        lowerPopR(MI, Alex::S0);
    }

    return true;
}

bool AlexInstrInfo::lowerStoreMem(MachineBasicBlock::iterator &MI, unsigned Src, unsigned Base, int Offset) const {
    llvm_unreachable("lower store mem ");
    return false;
}















































