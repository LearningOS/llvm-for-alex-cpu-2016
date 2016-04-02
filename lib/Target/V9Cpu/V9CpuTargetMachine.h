#ifndef V9CPU_TARGET_MACHINE_H
#define V9CPU_TARGET_MACHINE_H

#include "llvm/Target/TargetMachine.h"
#include "V9CpuSubtarget.h"

namespace llvm {

    class V9CpuTargetMachine : public LLVMTargetMachine {
        std::unique_ptr<TargetLoweringObjectFile> TLOF;
        V9CpuABIInfo ABI;
        V9CpuSubtarget DefaultSubtarget;

        mutable StringMap<std::unique_ptr<V9CpuSubtarget>> SubtargetMap;
    public:
        V9CpuTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                        StringRef FS, const TargetOptions &Options,
                        Reloc::Model RM, CodeModel::Model CM, CodeGenOpt::Level OL);
        ~V9CpuTargetMachine() override;
        TargetLoweringObjectFile *getObjFileLowering() const override {
            return TLOF.get();
        }

        const V9CpuSubtarget *getSubtargetImpl() const {
            return &DefaultSubtarget;
        }

        const V9CpuSubtarget *getSubtargetImpl(const Function &F) const override;

    };

    class V9CpuITargetMachine : public V9CpuTargetMachine {
        virtual void anchor();
    public:
        V9CpuITargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                            StringRef FS, const TargetOptions &Options,
                            Reloc::Model RM, CodeModel::Model CM,
                            CodeGenOpt::Level OL);
    };


} // end namespace llvm


#endif // V9CPU_TARGET_MACHINE_H