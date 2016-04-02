#ifndef LLVM_LIB_TARGET_V9CPU_MCTARGETDESC_V9CPUMCASMINFO_H
#define LLVM_LIB_TARGET_V9CPU_MCTARGETDESC_V9CPUMCASMINFO_H

#include "llvm/MC/MCAsmInfoELF.h"

namespace llvm {
    class Triple;

    class V9CpuMCAsmInfo : public MCAsmInfoELF {
        void anchor() override;
    public:
        V9CpuMCAsmInfo(const Triple &TheTriple);
    };

} // namespace llvm

#endif