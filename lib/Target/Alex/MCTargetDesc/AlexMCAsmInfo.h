
#ifndef LLVM_LIB_TARGET_ALEX_MCTARGETDESC_ALEXMCASMINFO_H
#define LLVM_LIB_TARGET_ALEX_MCTARGETDESC_ALEXMCASMINFO_H

#include "llvm/MC/MCAsmInfoELF.h"

namespace llvm {
    class Triple;

    class AlexMCAsmInfo : public MCAsmInfoELF {
        void anchor() override;
    public:
        AlexMCAsmInfo(const Triple &TheTriple);
    };

} // namespace llvm

#endif