#ifndef V9CPU_TARGET_MACHINE_H
#define V9CPU_TARGET_MACHINE_H

#include "llvm/Target/TargetMachine.h"

namespace llvm {

    class V9CpuSeriesTargetMachine : public LLVMTargetMachine {
    public:
        V9CpuSeriesTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                        StringRef FS, const TargetOptions &Options,
                        Reloc::Model RM, CodeModel::Model CM, CodeGenOpt::Level OL);
        ~V9CpuSeriesTargetMachine() override;
    };


} // end namespace llvm


#endif // V9CPU_TARGET_MACHINE_H