#ifndef LLVM_LIB_TARGET_V9CPU_V9CPUTARGETOBJECTFILE_H
#define LLVM_LIB_TARGET_V9CPU_V9CPUTARGETOBJECTFILE_H

#include "V9CpuTargetMachine.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"

namespace llvm {
    class V9CpuTargetMachine;
    class V9CpuTargetObjectFile : public TargetLoweringObjectFileELF {
        MCSection *SmallDataSection;
        MCSection *SmallBSSSection;
        const V9CpuTargetMachine *TM;
    public:

        void Initialize(MCContext &Ctx, const TargetMachine &TM) override;

    };
} // end namespace llvm

#endif