#ifndef LLVM_LIB_TARGET_V9Cpu_V9CpuSEREGISTERINFO_H
#define LLVM_LIB_TARGET_V9Cpu_V9CpuSEREGISTERINFO_H

#include "V9CpuRegisterInfo.h"
#include "V9CpuSubtarget.h"
#include "llvm/Target/TargetRegisterInfo.h"

namespace llvm {
class V9CpuSEInstrInfo;

class V9CpuSERegisterInfo : public V9CpuRegisterInfo {
public:
  V9CpuSERegisterInfo(const V9CpuSubtarget &Subtarget);

  //const TargetRegisterClass *intRegClass(unsigned Size) const override;
};

} // end namespace llvm

#endif