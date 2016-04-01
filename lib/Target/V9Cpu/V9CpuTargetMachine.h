#ifndef V9CPU_TARGET_MACHINE_H
#define V9CPU_TARGET_MACHINE_H

#include "V9CpuInstrInfo.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {

    class V9CpuSeriesTargetMachine : public LLVMTargetMachine {
    public:
        V9CpuSeriesTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                        StringRef FS, const TargetOptions &Options,
                        Reloc::Model RM, CodeModel::Model CM, CodeGenOpt::Level OL);
        ~V9CpuSeriesTargetMachine() override;

        const V9Subtarget *getSubtargetImpl(const Function &) const override {
            return &Subtarget;
        }

        // Pass Pipeline Configuration
        TargetPassConfig *createPassConfig(PassManagerBase &PM) override;
        TargetLoweringObjectFile *getObjFileLowering() const override {
            return TLOF.get();
        }
    };

    class V9CpuSubTargetMachine : public V9TargetMachine {
        virtual void anchor();
    public:
        V9CpuZeroTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                          StringRef FS, const TargetOptions &Options,
                          Reloc::Model RM, CodeModel::Model CM,
                          CodeGenOpt::Level OL);
    };

} // end namespace llvm


#endif // V9CPU_TARGET_MACHINE_H