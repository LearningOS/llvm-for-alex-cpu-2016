#include "V9CpuSERegisterInfo.h"

using namespace llvm;

#define DEBUG_TYPE "v9cpu-reg-info"

V9CpuSERegisterInfo::V9CpuSERegisterInfo(const V9CpuSubtarget &ST)
        : V9CpuRegisterInfo(ST) {}

//const TargetRegisterClass *
//V9CpuSERegisterInfo::intRegClass(unsigned Size) const {
//    return &V9Cpu::CPURegsRegClass;
//}