#ifndef LLVM_LIB_TARGET_V9CPU_V9CPUSEISELDAGTODAG_H
#define LLVM_LIB_TARGET_V9CPU_V9CPUSEISELDAGTODAG_H

#include "V9CpuISelDAGToDAG.h"

namespace llvm {

    class V9CpuSEDAGToDAGISel : public V9CpuDAGToDAGISel {

    public:
        explicit V9CpuSEDAGToDAGISel(V9CpuTargetMachine &TM) : V9CpuDAGToDAGISel(TM) {}

    private:

        bool runOnMachineFunction(MachineFunction &MF) override;

        std::pair<bool, SDNode*> selectNode(SDNode *Node) override;

        void processFunctionAfterISel(MachineFunction &MF) override;

    };

    FunctionPass *createV9CpuSEISelDag(V9CpuTargetMachine &TM);

}

#endif