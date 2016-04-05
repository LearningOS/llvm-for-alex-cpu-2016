#ifndef LLVM_LIB_TARGET_ALEX_ALEXTARGETOBJECTFILE_H
#define LLVM_LIB_TARGET_ALEX_ALEXTARGETOBJECTFILE_H

#include "AlexTargetMachine.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"

namespace llvm {
    class AlexTargetMachine;
    class AlexTargetObjectFile : public TargetLoweringObjectFileELF {
        MCSection *SmallDataSection;
        MCSection *SmallBSSSection;
        const AlexTargetMachine *TM;
    public:
        void Initialize(MCContext &Ctx, const TargetMachine &TM) override;
    };
} // end namespace llvm

#endif