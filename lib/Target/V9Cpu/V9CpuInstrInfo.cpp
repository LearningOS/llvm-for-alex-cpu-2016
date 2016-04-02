#include "V9CpuInstrInfo.h"

#include "V9CpuTargetMachine.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

#define GET_INSTRINFO_CTOR_DTOR
#include "V9CpuGenInstrInfo.inc"

// Pin the vtable to this file.
void V9CpuInstrInfo::anchor() {}

//@V9CpuInstrInfo {
V9CpuInstrInfo::V9CpuInstrInfo(const V9CpuSubtarget &STI)
        :
        Subtarget(STI) {}

const V9CpuInstrInfo *V9CpuInstrInfo::create(V9CpuSubtarget &STI) {
    return llvm::createV9CpuSEInstrInfo(STI);
}