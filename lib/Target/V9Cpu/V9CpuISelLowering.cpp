#include "V9CpuTargetMachine.h"
#include "V9CpuMachineFunction.h"
#include "V9CpuISelLowering.h"
#include "V9CpuSEISelLowering.h"

#include "V9CpuTargetObjectFile.h"
#include "V9CpuSubtarget.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/CodeGen/ValueTypes.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

#define DEBUG_TYPE "V9Cpu-lower"

//@3_1 1 {
const char *V9CpuTargetLowering::getTargetNodeName(unsigned Opcode) const {
    switch (Opcode) {
        case V9CpuISD::JmpLink:           return "V9CpuISD::JmpLink";
        case V9CpuISD::TailCall:          return "V9CpuISD::TailCall";
        case V9CpuISD::Hi:                return "V9CpuISD::Hi";
        case V9CpuISD::Lo:                return "V9CpuISD::Lo";
        case V9CpuISD::GPRel:             return "V9CpuISD::GPRel";
        case V9CpuISD::Ret:               return "V9CpuISD::Ret";
        case V9CpuISD::EH_RETURN:         return "V9CpuISD::EH_RETURN";
        case V9CpuISD::DivRem:            return "V9CpuISD::DivRem";
        case V9CpuISD::DivRemU:           return "V9CpuISD::DivRemU";
        case V9CpuISD::Wrapper:           return "V9CpuISD::Wrapper";
        default:                         return NULL;
    }
}
//@3_1 1 }

//@V9CpuTargetLowering {
V9CpuTargetLowering::V9CpuTargetLowering(const V9CpuTargetMachine &TM,
                                       const V9CpuSubtarget &STI)
        : TargetLowering(TM), Subtarget(STI), ABI(TM.getABI()) {

}

const V9CpuTargetLowering *V9CpuTargetLowering::create(const V9CpuTargetMachine &TM,
                                                     const V9CpuSubtarget &STI) {
    return llvm::createV9CpuSETargetLowering(TM, STI);
}

//===----------------------------------------------------------------------===//
//  Lower helper functions
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
//  Misc Lower Operation implementation
//===----------------------------------------------------------------------===//

#include "V9CpuGenCallingConv.inc"

static unsigned
addLiveIn(MachineFunction &MF, unsigned PReg, const TargetRegisterClass *RC)
{
    unsigned VReg = MF.getRegInfo().createVirtualRegister(RC);
    MF.getRegInfo().addLiveIn(PReg, VReg);
    return VReg;
}

//===----------------------------------------------------------------------===//
//@            Formal Arguments Calling Convention Implementation
//===----------------------------------------------------------------------===//

//@LowerFormalArguments {
/// LowerFormalArguments - transform physical registers into virtual registers
/// and generate load operations for arguments places on the stack.
SDValue
V9CpuTargetLowering::LowerFormalArguments(SDValue Chain,
                                         CallingConv::ID CallConv,
                                         bool IsVarArg,
                                         const SmallVectorImpl<ISD::InputArg> &Ins,
                                         SDLoc DL, SelectionDAG &DAG,
                                         SmallVectorImpl<SDValue> &InVals)
const {

    MachineFunction &MF = DAG.getMachineFunction();
    SmallVector<CCValAssign, 16> ArgLocs;
    CCState CCInfo(CallConv, IsVarArg, DAG.getMachineFunction(), ArgLocs,
                    *DAG.getContext());
    CCInfo.AnalyzeFormalArguments(Ins, CC_V9Cpu);
    const unsigned ArgArea = 128;

    for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
        //@2 }
        CCValAssign &VA = ArgLocs[i];
        if (VA.isRegLoc()) {
            unsigned VReg = MF.addLiveIn(VA.getLocReg(),
              getRegClassFor(VA.getLocVT()));

            unsigned ArgReg = VA.getLocReg();
            MVT RegVT = VA.getLocVT();
            const TargetRegisterClass *RC = getRegClassFor(RegVT);
            unsigned Reg = addLiveIn(DAG.getMachineFunction(), ArgReg, RC);
            SDValue ArgValue = DAG.getCopyFromReg(Chain, DL, Reg, RegVT);

            InVals.push_back(ArgValue);
        }
    }

    return Chain;
}
// @LowerFormalArguments }

//===----------------------------------------------------------------------===//
//@              Return Value Calling Convention Implementation
//===----------------------------------------------------------------------===//

SDValue
V9CpuTargetLowering::LowerReturn(SDValue Chain,
                                CallingConv::ID CallConv, bool IsVarArg,
                                const SmallVectorImpl<ISD::OutputArg> &Outs,
                                const SmallVectorImpl<SDValue> &OutVals,
                                SDLoc DL, SelectionDAG &DAG) const {
 SmallVector<CCValAssign, 16> RVLocs;
  MachineFunction &MF = DAG.getMachineFunction();

  // CCState - Info about the registers and stack slot.
  CCState CCInfo(CallConv, IsVarArg, MF, RVLocs,
                 *DAG.getContext());
  V9CpuCC V9CpuCCInfo(CallConv, ABI.IsO32(),
                    CCInfo);

  // Analyze return values.
  V9CpuCCInfo.analyzeReturn(Outs, false,
                           MF.getFunction()->getReturnType());

  SDValue Flag;
  SmallVector<SDValue, 4> RetOps(1, Chain);

  // Copy the result values into the output registers.
  for (unsigned i = 0; i != RVLocs.size(); ++i) {
    SDValue Val = OutVals[i];
    CCValAssign &VA = RVLocs[i];
    assert(VA.isRegLoc() && "Can only return in registers!");

    if (RVLocs[i].getValVT() != RVLocs[i].getLocVT())
      Val = DAG.getNode(ISD::BITCAST, DL, RVLocs[i].getLocVT(), Val);

    Chain = DAG.getCopyToReg(Chain, DL, VA.getLocReg(), Val, Flag);

    // Guarantee that all emitted copies are stuck together with flags.
    Flag = Chain.getValue(1);
    RetOps.push_back(DAG.getRegister(VA.getLocReg(), VA.getLocVT()));
  }

//@Ordinary struct type: 2 {
  // The V9Cpu ABIs for returning structs by value requires that we copy
  // the sret argument into $v0 for the return. We saved the argument into
  // a virtual register in the entry block, so now we copy the value out
  // and into $v0.
  if (MF.getFunction()->hasStructRetAttr()) {
    V9CpuFunctionInfo *V9CpuFI = MF.getInfo<V9CpuFunctionInfo>();
    unsigned Reg = V9CpuFI->getSRetReturnReg();

    if (!Reg)
      llvm_unreachable("sret virtual register not created in the entry block");
    SDValue Val =
        DAG.getCopyFromReg(Chain, DL, Reg, getPointerTy(DAG.getDataLayout()));
    unsigned V0 = V9Cpu::R6;

    Chain = DAG.getCopyToReg(Chain, DL, V0, Val, Flag);
    Flag = Chain.getValue(1);
    RetOps.push_back(DAG.getRegister(V0, getPointerTy(DAG.getDataLayout())));
  }
//@Ordinary struct type: 2 }

  RetOps[0] = Chain;  // Update chain.

  // Add the flag if we have it.
  if (Flag.getNode())
    RetOps.push_back(Flag);

  // Return on V9Cpu is always a "ret $lr"
  return DAG.getNode(V9CpuISD::Ret, DL, MVT::Other, RetOps);
}

template<typename Ty>
void V9CpuTargetLowering::V9CpuCC::
analyzeReturn(const SmallVectorImpl<Ty> &RetVals, bool IsSoftFloat,
              const SDNode *CallNode, const Type *RetTy) const {
  CCAssignFn *Fn;

  Fn = RetCC_V9Cpu;

  for (unsigned I = 0, E = RetVals.size(); I < E; ++I) {
    MVT VT = RetVals[I].VT;
    ISD::ArgFlagsTy Flags = RetVals[I].Flags;
    MVT RegVT = this->getRegVT(VT, RetTy, CallNode, IsSoftFloat);

    if (Fn(I, VT, RegVT, CCValAssign::Full, Flags, this->CCInfo)) {
#ifndef NDEBUG
      dbgs() << "Call result #" << I << " has unhandled type "
             << EVT(VT).getEVTString() << '\n';
#endif
      llvm_unreachable(nullptr);
    }
  }
}
V9CpuTargetLowering::V9CpuCC::V9CpuCC(
        CallingConv::ID CC, bool IsO32_, CCState &Info,
        V9CpuCC::SpecialCallingConvType SpecialCallingConv_)
        : CCInfo(Info), CallConv(CC), IsO32(IsO32_) {
    // Pre-allocate reserved argument area.
    CCInfo.AllocateStack(reservedArgArea(), 1);
}
void V9CpuTargetLowering::V9CpuCC::
analyzeCallResult(const SmallVectorImpl<ISD::InputArg> &Ins, bool IsSoftFloat,
                  const SDNode *CallNode, const Type *RetTy) const {
  analyzeReturn(Ins, IsSoftFloat, CallNode, RetTy);
}

void V9CpuTargetLowering::V9CpuCC::
analyzeReturn(const SmallVectorImpl<ISD::OutputArg> &Outs, bool IsSoftFloat,
              const Type *RetTy) const {
  analyzeReturn(Outs, IsSoftFloat, nullptr, RetTy);
}
//
//V9CpuTargetLowering::V9CpuTargetLowering(V9CpuTargetMachine &TM)
//  : TargetLowering(TM, new V9CpuTargetObjectFile()),
//    Subtarget(&TM.getSubtarget<V9CpuSubtarget>()) {
//
////- Set .align 2
//// It will emit .align 2 later
//  setMinFunctionAlignment(2);
//
//// must, computeRegisterProperties - Once all of the register classes are
////  added, this allows us to compute derived properties we expose.
//  computeRegisterProperties();
//}
