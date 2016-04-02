#include "V9CpuMachineFunction.h"
#include "V9CpuSEISelLowering.h"

#include "V9CpuRegisterInfo.h"
#include "V9CpuTargetMachine.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetInstrInfo.h"

using namespace llvm;

#define DEBUG_TYPE "v9cpu-isel"

static cl::opt<bool>
        EnableV9CpuTailCalls("enable-v9cpu-tail-calls", cl::Hidden,
                            cl::desc("V9CPU: Enable tail calls."), cl::init(false));

//@V9CpuSETargetLowering {
V9CpuSETargetLowering::V9CpuSETargetLowering(const V9CpuTargetMachine &TM,
                                           const V9CpuSubtarget &STI)
        : V9CpuTargetLowering(TM, STI) {
//@V9CpuSETargetLowering body {
    // Set up the register classes
    addRegisterClass(MVT::i32, &V9Cpu::ARegsRegClass);

// must, computeRegisterProperties - Once all of the register classes are 
//  added, this allows us to compute derived properties we expose.
    computeRegisterProperties(Subtarget.getRegisterInfo());
}

SDValue V9CpuSETargetLowering::LowerOperation(SDValue Op,
                                             SelectionDAG &DAG) const {

    return V9CpuTargetLowering::LowerOperation(Op, DAG);
}

const V9CpuTargetLowering *
llvm::createV9CpuSETargetLowering(const V9CpuTargetMachine &TM,
                                 const V9CpuSubtarget &STI) {
    return new V9CpuSETargetLowering(TM, STI);
}