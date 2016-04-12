#include "AlexTargetMachine.h"
#include "AlexISelLowering.h"
#include "AlexISelDAGToDAG.h"
#include "AlexRegisterInfo.h"
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
#include "AlexGenCallingConv.inc"

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

AlexTargetLowering::AlexTargetLowering(const AlexTargetMachine *targetMachine,
                                         const AlexSubtarget *subtarget,
                                       const AlexRegisterInfo* registerInfo)
        : TargetLowering(*targetMachine), subtarget(subtarget) {
    // disable dag nodes here
    setOperationAction(ISD::BR_CC,             MVT::i32, Expand);
    addRegisterClass(MVT::i32, &Alex::Int32RegsRegClass);
    computeRegisterProperties(registerInfo);
}

SDValue AlexTargetLowering::LowerFormalArguments(SDValue Chain, CallingConv::ID CallConv, bool IsVarArg,
                                                 const SmallVectorImpl<ISD::InputArg> &Ins, SDLoc dl, SelectionDAG &DAG,
                                                 SmallVectorImpl<SDValue> &InVals) const {
    return Chain;
}

SDValue AlexTargetLowering::LowerReturn(SDValue chain, CallingConv::ID CallConv, bool isVarArg,
                                        const SmallVectorImpl<ISD::OutputArg> &outs,
                                        const SmallVectorImpl<SDValue> &outVals,
                                        SDLoc dl,
                                        SelectionDAG &dag) const {
   /* SmallVector<CCValAssign, 16> RVLocs;
    MachineFunction &MF = dag.getMachineFunction();

    // CCState - Info about the registers and stack slot.
    CCState CCInfo(CallConv, isVarArg, MF, RVLocs,
                   *dag.getContext());
    ///Cpu0CC Cpu0CCInfo(CallConv, ABI.IsO32(),
    //                  CCInfo);

    // Analyze return values.
   // Cpu0CCInfo.analyzeReturn(outs, false, // use soft float
    //                         MF.getFunction()->getReturnType());

    SDValue Flag;
    SmallVector<SDValue, 4> RetOps(1, chain);

    // Copy the result values into the output registers.
    for (unsigned i = 0; i != RVLocs.size(); ++i) {
        SDValue Val = outVals[i];
        CCValAssign &VA = RVLocs[i];
        assert(VA.isRegLoc() && "Can only return in registers!");

        //if (RVLocs[i].getValVT() != RVLocs[i].getLocVT())
        //    Val = DAG.getNode(ISD::BITCAST, DL, RVLocs[i].getLocVT(), Val);

        chain = dag.getCopyToReg(chain, dl, VA.getLocReg(), Val, Flag);

        // Guarantee that all emitted copies are stuck together with flags.
        Flag = chain.getValue(1);
        RetOps.push_back(dag.getRegister(VA.getLocReg(), VA.getLocVT()));
    }

    RetOps[0] = chain;  // Update chain.

    // Add the flag if we have it.
    //if (Flag.getNode())
    //    RetOps.push_back(Flag);

    // Return on Cpu0 is always a "ret $lr"
    //return DAG.getNode(Cpu0ISD::Ret, DL, MVT::Other, RetOps);
    // R1 =

    //return dag.getNode(Alex::ADDIU, dl, MVT::Other,
    //                  chain, dag.getRegister(Alex::LR, MVT::i32));*/
    return dag.getNode(AlexISD::Ret, dl, MVT::Other,
                       chain, dag.getRegister(Alex::LR, MVT::i32));
}






