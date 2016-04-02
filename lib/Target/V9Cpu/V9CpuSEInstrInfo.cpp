#include "V9CpuSEInstrInfo.h"
#include "V9CpuMachineFunction.h"
#include "V9CpuTargetMachine.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

V9CpuSEInstrInfo::V9CpuSEInstrInfo(const V9CpuSubtarget &STI)
    : V9CpuInstrInfo(STI),
      RI(STI) {}

const V9CpuRegisterInfo &V9CpuSEInstrInfo::getRegisterInfo() const {
  return RI;
}

const V9CpuInstrInfo *llvm::createV9CpuSEInstrInfo(const V9CpuSubtarget &STI) {
  return new V9CpuSEInstrInfo(STI);
}
