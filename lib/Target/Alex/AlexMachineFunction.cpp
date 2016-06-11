#include "AlexMachineFunction.h"

#include "AlexInstrInfo.h"
#include "AlexTargetMachine.h"
#include "llvm/IR/Function.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"

using namespace llvm;

bool FixGlobalBaseReg;

// class AlexCallEntry.
//AlexCallEntry::AlexCallEntry(StringRef N) {
//#ifndef NDEBUG
//  Name = N;
//  Val = nullptr;
//#endif
//}
//
//AlexCallEntry::AlexCallEntry(const GlobalValue *V) {
//#ifndef NDEBUG
//  Val = V;
//#endif
//}

bool AlexCallEntry::isConstant(const MachineFrameInfo *) const {
    return false;
}

bool AlexCallEntry::isAliased(const MachineFrameInfo *) const {
    return false;
}

bool AlexCallEntry::mayAlias(const MachineFrameInfo *) const {
    return false;
}

void AlexCallEntry::printCustom(raw_ostream &O) const {
    O << "AlexCallEntry: ";
}

AlexCallEntry::AlexCallEntry(StringRef N) :PseudoSourceValue(GlobalValueCallEntry) {
    Name = N;
    Val = nullptr;
}

AlexCallEntry::AlexCallEntry(const GlobalValue *V) :PseudoSourceValue(GlobalValueCallEntry) {
    Val = V;
}


AlexFunctionInfo::AlexFunctionInfo(MachineFunction& MF)
        : MF(MF),  SRetReturnReg(0),
          VarArgsFrameIndex(0), DynAllocFI(0), OutArgFIRange(std::make_pair(-1, 0)),
          MaxCallFrameSize(0) {}
AlexFunctionInfo::~AlexFunctionInfo() {}

void AlexFunctionInfo::anchor() { }

MachinePointerInfo AlexFunctionInfo::callPtrInfo(StringRef Name) {
    std::unique_ptr<const AlexCallEntry> &E = ExternalCallEntries[Name];

    if (!E)
        E = llvm::make_unique<AlexCallEntry>(Name);

    return MachinePointerInfo(E.get());
}

MachinePointerInfo AlexFunctionInfo::callPtrInfo(const GlobalValue *Val) {
    std::unique_ptr<const AlexCallEntry> &E = GlobalCallEntries[Val];

    if (!E)
        E = llvm::make_unique<AlexCallEntry>(Val);

    return MachinePointerInfo(E.get());
}