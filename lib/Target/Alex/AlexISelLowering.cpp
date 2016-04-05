#include "AlexTargetMachine.h"
#include "AlexISelLowering.h"
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
                                         const AlexSubtarget *subtarget)
        : TargetLowering(*targetMachine), subtarget(subtarget) {
    // disable dag nodes here
    setOperationAction(ISD::BR_CC,             MVT::i32, Expand);
    addRegisterClass(MVT::i32, &Alex::Int32RegsRegClass);
    computeRegisterProperties(subtarget->getRegisterInfo());
}

SDValue AlexTargetLowering::LowerFormalArguments(SDValue Chain, CallingConv::ID CallConv, bool IsVarArg,
                                                 const SmallVectorImpl<ISD::InputArg> &Ins, SDLoc dl, SelectionDAG &DAG,
                                                 SmallVectorImpl<SDValue> &InVals) const {
    return Chain;
}

SDValue AlexTargetLowering::LowerReturn(SDValue Chain, CallingConv::ID CallConv, bool IsVarArg,
                                        const SmallVectorImpl<ISD::OutputArg> &Outs,
                                        const SmallVectorImpl<SDValue> &OutVals, SDLoc dl, SelectionDAG &DAG) const {
    return Chain;
}






