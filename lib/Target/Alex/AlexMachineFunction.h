#ifndef LLVM_LIB_TARGET_ALEX_ALEXMACHINEFUNCTION_H
#define LLVM_LIB_TARGET_ALEX_ALEXMACHINEFUNCTION_H

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
    class AlexCallEntry : public PseudoSourceValue {
    public:
        explicit AlexCallEntry(StringRef N);
        explicit AlexCallEntry(const GlobalValue *V);
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
/// AlexFunctionInfo - This class is derived from MachineFunction private
/// Alex target-specific information for each MachineFunction.
    class AlexFunctionInfo : public MachineFunctionInfo {
    public:
        AlexFunctionInfo(MachineFunction& MF);

        ~AlexFunctionInfo();


        int getVarArgsFrameIndex() const { return VarArgsFrameIndex; }
        void setVarArgsFrameIndex(int Index) { VarArgsFrameIndex = Index; }
        unsigned getSRetReturnReg() const { return SRetReturnReg; }
        void setSRetReturnReg(unsigned Reg) { SRetReturnReg = Reg; }
        bool hasByvalArg() const { return HasByvalArg; }
        void setFormalArgInfo(unsigned Size, bool HasByval) {
            IncomingArgSize = Size;
            HasByvalArg = HasByval;
        }

        MachinePointerInfo callPtrInfo(StringRef Name);
        MachinePointerInfo callPtrInfo(const GlobalValue *Val);

        unsigned getIncomingArgSize() const { return IncomingArgSize; }
    private:
        bool HasByvalArg;
        unsigned IncomingArgSize;
        virtual void anchor();
        unsigned SRetReturnReg;
        MachineFunction& MF;

        /// VarArgsFrameIndex - FrameIndex for start of varargs area.
        int VarArgsFrameIndex;

        unsigned MaxCallFrameSize;

        /// AlexCallEntry maps.
        StringMap<std::unique_ptr<const AlexCallEntry>> ExternalCallEntries;
        ValueMap<const GlobalValue *, std::unique_ptr<const AlexCallEntry>>
                GlobalCallEntries;

    };
//@1 }

} // end of namespace llvm

#endif // ALEX_MACHINE_FUNCTION_INFO_H