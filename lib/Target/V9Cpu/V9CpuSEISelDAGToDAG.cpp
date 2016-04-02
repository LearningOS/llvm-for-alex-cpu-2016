#include "V9CpuSEISelDAGToDAG.h"

//#include "MCTargetDesc/V9CpuBaseInfo.h"
#include "V9Cpu.h"
#include "V9CpuMachineFunction.h"
#include "V9CpuRegisterInfo.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAGNodes.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
using namespace llvm;

#define DEBUG_TYPE "v9cpu-isel"

bool V9CpuSEDAGToDAGISel::runOnMachineFunction(MachineFunction &MF) {
  Subtarget = &static_cast<const V9CpuSubtarget &>(MF.getSubtarget());
  return V9CpuDAGToDAGISel::runOnMachineFunction(MF);
}

void V9CpuSEDAGToDAGISel::processFunctionAfterISel(MachineFunction &MF) {
}

//@selectNode
std::pair<bool, SDNode*> V9CpuSEDAGToDAGISel::selectNode(SDNode *Node) {
  unsigned Opcode = Node->getOpcode();
  SDLoc DL(Node);

  ///
  // Instruction Selection not handled by the auto-generated
  // tablegen selection should be handled here.
  ///
  SDNode *Result;

  ///
  // Instruction Selection not handled by the auto-generated
  // tablegen selection should be handled here.
  ///
  EVT NodeTy = Node->getValueType(0);
  unsigned MultOpc;

  switch(Opcode) {
  default: break;

  }

  return std::make_pair(false, nullptr);
}

FunctionPass *llvm::createV9CpuSEISelDag(V9CpuTargetMachine &TM) {
  return new V9CpuSEDAGToDAGISel(TM);
}