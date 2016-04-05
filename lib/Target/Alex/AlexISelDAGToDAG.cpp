
#include "AlexISelDAGToDAG.h"
#include "AlexRegisterInfo.h"
#include "AlexTargetMachine.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
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

#define DEBUG_TYPE "Alex-isel"

//===----------------------------------------------------------------------===//
// Instruction Selector Implementation
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// AlexDAGToDAGISel - Alex specific code to select Alex machine
// instructions for SelectionDAG operations.
//===----------------------------------------------------------------------===//

bool AlexDAGToDAGISel::runOnMachineFunction(MachineFunction &MF) {
    bool Ret = SelectionDAGISel::runOnMachineFunction(MF);

    return Ret;
}

std::pair<bool, SDNode*> AlexDAGToDAGISel::selectNode(SDNode *Node) {
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

SDNode* AlexDAGToDAGISel::Select(SDNode *Node) {

    unsigned Opcode = Node->getOpcode();

    // Dump information about the Node being selected
    DEBUG(errs() << "Selecting: "; Node->dump(CurDAG); errs() << "\n");

    // If we have a custom node, we already have selected!
    if (Node->isMachineOpcode()) {
        DEBUG(errs() << "== "; Node->dump(CurDAG); errs() << "\n");
        Node->setNodeId(-1);
        return nullptr;
    }

    // See if subclasses can handle this node.
    std::pair<bool, SDNode*> Ret = selectNode(Node);

    if (Ret.first)
        return Ret.second;

    switch(Opcode) {
        default: break;
        case ISD::LOAD:
        case ISD::STORE:
            break;
    }

    // Select the default instruction
    SDNode *ResNode = SelectCode(Node);

    DEBUG(errs() << "=> ");
    if (ResNode == nullptr || ResNode == Node)
        DEBUG(Node->dump(CurDAG));
    else
        DEBUG(ResNode->dump(CurDAG));
    DEBUG(errs() << "\n");
    return ResNode;
}