#ifndef LLVM_LIB_TARGET_V9CPU_V9CPUMACHINEFUNCTION_H
#define LLVM_LIB_TARGET_V9CPU_V9CPUMACHINEFUNCTION_H

#include "llvm/ADT/StringMap.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineMemOperand.h"
#include "llvm/CodeGen/PseudoSourceValue.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/ValueMap.h"
#include "llvm/Target/TargetFrameLowering.h"
#include "llvm/Target/TargetMachine.h"
#include <map>
#include <string>
#include <utility>

namespace llvm {

/// \brief A class derived from PseudoSourceValue that represents a GOT entry
/// resolved by lazy-binding.
    class V9CpuCallEntry : public PseudoSourceValue {
    public:
        explicit V9CpuCallEntry(StringRef N);
        explicit V9CpuCallEntry(const GlobalValue *V);
        bool isConstant(const MachineFrameInfo *) const override;
        bool isAliased(const MachineFrameInfo *) const override;
        bool mayAlias(const MachineFrameInfo *) const override;

    private:
        void printCustom(raw_ostream &O) const override;
#ifndef NDEBUG
        std::string Name;
        const GlobalValue *Val;
#endif
    };

//@1 {
/// V9CpuFunctionInfo - This class is derived from MachineFunction private
/// V9Cpu target-specific information for each MachineFunction.
    class V9CpuFunctionInfo : public MachineFunctionInfo {
    public:
        V9CpuFunctionInfo(MachineFunction& MF);

        ~V9CpuFunctionInfo();

        int getVarArgsFrameIndex() const { return VarArgsFrameIndex; }
        void setVarArgsFrameIndex(int Index) { VarArgsFrameIndex = Index; }

    private:
        virtual void anchor();

        MachineFunction& MF;

        /// VarArgsFrameIndex - FrameIndex for start of varargs area.
        int VarArgsFrameIndex;

        unsigned MaxCallFrameSize;

        /// V9CpuCallEntry maps.
        StringMap<std::unique_ptr<const V9CpuCallEntry>> ExternalCallEntries;
        ValueMap<const GlobalValue *, std::unique_ptr<const V9CpuCallEntry>>
                GlobalCallEntries;
    };
//@1 }

} // end of namespace llvm

#endif // V9CPU_MACHINE_FUNCTION_INFO_H