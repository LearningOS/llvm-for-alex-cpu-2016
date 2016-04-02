#include "V9CpuISelLowering.h"
#include "V9CpuSEISelLowering.h"
#include "V9CpuTargetMachine.h"
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
    return DAG.getNode(V9CpuISD::Ret, DL, MVT::Other,
                       Chain, DAG.getRegister(V9Cpu::A, MVT::i32));
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