#ifndef LLVM_LIB_TARGET_V9CPU_V9CPUSEINSTRINFO_H
#define LLVM_LIB_TARGET_V9CPU_V9CPUSEINSTRINFO_H


#include "V9CpuInstrInfo.h"
#include "V9CpuMachineFunction.h"
#include "V9CpuSERegisterInfo.h"

namespace llvm {

    class V9CpuSEInstrInfo : public V9CpuInstrInfo {
        const V9CpuSERegisterInfo RI;

    public:
        explicit V9CpuSEInstrInfo(const V9CpuSubtarget &STI);

        const V9CpuRegisterInfo &getRegisterInfo() const override;
    };
}

#endif