#include "AlexTargetMachine.h"
#include "AlexISelLowering.h"
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

#define DEBUG_TYPE "Alex-lower"

//@3_1 1 {
const char *AlexTargetLowering::getTargetNodeName(unsigned Opcode) const {
    switch (Opcode) {
        case AlexISD::JmpLink:           return "AlexISD::JmpLink";
        case AlexISD::TailCall:          return "AlexISD::TailCall";
        case AlexISD::Hi:                return "AlexISD::Hi";
        case AlexISD::Lo:                return "AlexISD::Lo";
        case AlexISD::GPRel:             return "AlexISD::GPRel";
        case AlexISD::Ret:               return "AlexISD::Ret";
        case AlexISD::EH_RETURN:         return "AlexISD::EH_RETURN";
        case AlexISD::DivRem:            return "AlexISD::DivRem";
        case AlexISD::DivRemU:           return "AlexISD::DivRemU";
        case AlexISD::Wrapper:           return "AlexISD::Wrapper";
        default:                         return NULL;
    }
}
//@3_1 1 }

//@AlexTargetLowering {
AlexTargetLowering::AlexTargetLowering(const AlexTargetMachine *targetMachine,
                                         const AlexSubtarget *subtarget)
        : TargetLowering(*targetMachine), subtarget(subtarget) {
    setOperationAction(ISD::BR_CC,             MVT::i32, Expand);
}

//===----------------------------------------------------------------------===//
//  Lower helper functions
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
//  Misc Lower Operation implementation
//===----------------------------------------------------------------------===//

#include "AlexGenCallingConv.inc"

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
//SDValue
//AlexTargetLowering::LowerFormalArguments(SDValue Chain,
//                                          CallingConv::ID CallConv,
//                                          bool IsVarArg,
//                                          const SmallVectorImpl<ISD::InputArg> &Ins,
//                                          SDLoc DL, SelectionDAG &DAG,
//                                          SmallVectorImpl<SDValue> &InVals)
//const {
//
//    MachineFunction &MF = DAG.getMachineFunction();
//    SmallVector<CCValAssign, 16> ArgLocs;
//    CCState CCInfo(CallConv, IsVarArg, DAG.getMachineFunction(), ArgLocs,
//                   *DAG.getContext());
//    CCInfo.AnalyzeFormalArguments(Ins, CC_Alex);
//    const unsigned ArgArea = 128;
//
//    for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
//        //@2 }
//        CCValAssign &VA = ArgLocs[i];
//        if (VA.isRegLoc()) {
//            unsigned VReg = MF.addLiveIn(VA.getLocReg(),
//                                         getRegClassFor(VA.getLocVT()));
//
//            unsigned ArgReg = VA.getLocReg();
//            MVT RegVT = VA.getLocVT();
//            const TargetRegisterClass *RC = getRegClassFor(RegVT);
//            unsigned Reg = addLiveIn(DAG.getMachineFunction(), ArgReg, RC);
//            SDValue ArgValue = DAG.getCopyFromReg(Chain, DL, Reg, RegVT);
//
//            InVals.push_back(ArgValue);
//        }
//    }
//
//    return Chain;
//}
//// @LowerFormalArguments }
//
////===----------------------------------------------------------------------===//
////@              Return Value Calling Convention Implementation
////===----------------------------------------------------------------------===//
//
//SDValue
//AlexTargetLowering::LowerReturn(SDValue Chain,
//                                 CallingConv::ID CallConv, bool IsVarArg,
//                                 const SmallVectorImpl<ISD::OutputArg> &Outs,
//                                 const SmallVectorImpl<SDValue> &OutVals,
//                                 SDLoc DL, SelectionDAG &DAG) const {
//    SmallVector<CCValAssign, 16> RVLocs;
//    MachineFunction &MF = DAG.getMachineFunction();
//
//    // CCState - Info about the registers and stack slot.
//    CCState CCInfo(CallConv, IsVarArg, MF, RVLocs,
//                   *DAG.getContext());
//    AlexCC AlexCCInfo(CallConv, ABI.IsO32(),
//                        CCInfo);
//
//    // Analyze return values.
//    AlexCCInfo.analyzeReturn(Outs, false,
//                              MF.getFunction()->getReturnType());
//
//    SDValue Flag;
//    SmallVector<SDValue, 4> RetOps(1, Chain);
//
//    // Copy the result values into the output registers.
//    for (unsigned i = 0; i != RVLocs.size(); ++i) {
//        SDValue Val = OutVals[i];
//        CCValAssign &VA = RVLocs[i];
//        assert(VA.isRegLoc() && "Can only return in registers!");
//
//        if (RVLocs[i].getValVT() != RVLocs[i].getLocVT())
//            Val = DAG.getNode(ISD::BITCAST, DL, RVLocs[i].getLocVT(), Val);
//
//        Chain = DAG.getCopyToReg(Chain, DL, VA.getLocReg(), Val, Flag);
//
//        // Guarantee that all emitted copies are stuck together with flags.
//        Flag = Chain.getValue(1);
//        RetOps.push_back(DAG.getRegister(VA.getLocReg(), VA.getLocVT()));
//    }
//
////@Ordinary struct type: 2 {
//    // The Alex ABIs for returning structs by value requires that we copy
//    // the sret argument into $v0 for the return. We saved the argument into
//    // a virtual register in the entry block, so now we copy the value out
//    // and into $v0.
//    if (MF.getFunction()->hasStructRetAttr()) {
//        AlexFunctionInfo *AlexFI = MF.getInfo<AlexFunctionInfo>();
//        unsigned Reg = AlexFI->getSRetReturnReg();
//
//        if (!Reg)
//            llvm_unreachable("sret virtual register not created in the entry block");
//        SDValue Val =
//                DAG.getCopyFromReg(Chain, DL, Reg, getPointerTy(DAG.getDataLayout()));
//        unsigned V0 = Alex::R6;
//
//        Chain = DAG.getCopyToReg(Chain, DL, V0, Val, Flag);
//        Flag = Chain.getValue(1);
//        RetOps.push_back(DAG.getRegister(V0, getPointerTy(DAG.getDataLayout())));
//    }
////@Ordinary struct type: 2 }
//
//    RetOps[0] = Chain;  // Update chain.
//
//    // Add the flag if we have it.
//    if (Flag.getNode())
//        RetOps.push_back(Flag);
//
//    // Return on Alex is always a "ret $lr"
//    return DAG.getNode(AlexISD::Ret, DL, MVT::Other, RetOps);
//}
//
//template<typename Ty>
//void AlexTargetLowering::AlexCC::
//analyzeReturn(const SmallVectorImpl<Ty> &RetVals, bool IsSoftFloat,
//              const SDNode *CallNode, const Type *RetTy) const {
//    CCAssignFn *Fn;
//
//    Fn = RetCC_Alex;
//
//    for (unsigned I = 0, E = RetVals.size(); I < E; ++I) {
//        MVT VT = RetVals[I].VT;
//        ISD::ArgFlagsTy Flags = RetVals[I].Flags;
//        MVT RegVT = this->getRegVT(VT, RetTy, CallNode, IsSoftFloat);
//
//        if (Fn(I, VT, RegVT, CCValAssign::Full, Flags, this->CCInfo)) {
//#ifndef NDEBUG
//            dbgs() << "Call result #" << I << " has unhandled type "
//            << EVT(VT).getEVTString() << '\n';
//#endif
//            llvm_unreachable(nullptr);
//        }
//    }
//}
//AlexTargetLowering::AlexCC::AlexCC(
//        CallingConv::ID CC, bool IsO32_, CCState &Info,
//        AlexCC::SpecialCallingConvType SpecialCallingConv_)
//        : CCInfo(Info), CallConv(CC), IsO32(IsO32_) {
//    // Pre-allocate reserved argument area.
//    CCInfo.AllocateStack(reservedArgArea(), 1);
//}
//void AlexTargetLowering::AlexCC::
//analyzeCallResult(const SmallVectorImpl<ISD::InputArg> &Ins, bool IsSoftFloat,
//                  const SDNode *CallNode, const Type *RetTy) const {
//    analyzeReturn(Ins, IsSoftFloat, CallNode, RetTy);
//}
//
//void AlexTargetLowering::AlexCC::
//analyzeReturn(const SmallVectorImpl<ISD::OutputArg> &Outs, bool IsSoftFloat,
//              const Type *RetTy) const {
//    analyzeReturn(Outs, IsSoftFloat, nullptr, RetTy);
//}
