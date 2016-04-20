#ifndef LLVM_LIB_TARGET_Alex_AlexISELDAGTODAG_H
#define LLVM_LIB_TARGET_Alex_AlexISELDAGTODAG_H

#include "AlexTargetMachine.h"
#include "AlexInstrInfo.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/Debug.h"

namespace llvm {

class AlexDAGToDAGISel : public SelectionDAGISel {
public:
  explicit AlexDAGToDAGISel(AlexTargetMachine &TM)
                : SelectionDAGISel(TM), Subtarget(nullptr) { }

  const char *getPassName() const override {
      return "Alex DAG->DAG Pattern Instruction Selection";
  }

  bool runOnMachineFunction(MachineFunction &MF) override;

protected:
  const AlexSubtarget *Subtarget;
private:
#include "AlexGenDAGISel.inc"

  SDNode *Select(SDNode *N) override;

  // Complex Pattern.
  bool SelectAddr(SDNode *Parent, SDValue Addr, SDValue &Base, SDValue &Offset);

  inline SDValue getImm(const SDNode *Node, unsigned Imm) {
      return CurDAG->getTargetConstant(Imm, SDLoc(Node), Node->getValueType(0));
  }

  bool SelectInlineAsmMemoryOperand(const SDValue &Op, unsigned ConstraintID,
                                    std::vector<SDValue> &OutOps);
};
}

#endif