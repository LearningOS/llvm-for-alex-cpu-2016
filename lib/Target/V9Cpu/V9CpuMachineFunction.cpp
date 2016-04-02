#include "V9CpuMachineFunction.h"

#include "V9CpuInstrInfo.h"
#include "V9CpuSubtarget.h"
#include "llvm/IR/Function.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"

using namespace llvm;

bool FixGlobalBaseReg;

// class V9CpuCallEntry.
//V9CpuCallEntry::V9CpuCallEntry(StringRef N) {
//#ifndef NDEBUG
//  Name = N;
//  Val = nullptr;
//#endif
//}
//
//V9CpuCallEntry::V9CpuCallEntry(const GlobalValue *V) {
//#ifndef NDEBUG
//  Val = V;
//#endif
//}

bool V9CpuCallEntry::isConstant(const MachineFrameInfo *) const {
  return false;
}

bool V9CpuCallEntry::isAliased(const MachineFrameInfo *) const {
  return false;
}

bool V9CpuCallEntry::mayAlias(const MachineFrameInfo *) const {
  return false;
}

void V9CpuCallEntry::printCustom(raw_ostream &O) const {
  O << "V9CpuCallEntry: ";
}
V9CpuFunctionInfo::V9CpuFunctionInfo(MachineFunction& MF)
                : MF(MF),
                  VarArgsFrameIndex(0),
                  MaxCallFrameSize(0) {}
V9CpuFunctionInfo::~V9CpuFunctionInfo() {}

void V9CpuFunctionInfo::anchor() { }