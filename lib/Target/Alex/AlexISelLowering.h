#ifndef LLVM_LIB_TARGET_ALEX_ALEXISELLOWERING_H
#define LLVM_LIB_TARGET_ALEX_ALEXISELLOWERING_H

#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/IR/Function.h"
#include "llvm/Target/TargetLowering.h"
#include "AlexTargetMachine.h"
#include <deque>

namespace llvm {
    namespace AlexISD {
        enum NodeType {
            // Start the numbering from where ISD NodeType finishes.
                    FIRST_NUMBER = ISD::BUILTIN_OP_END,

            // Jump and link (call)
                    JmpLink,

            // Tail call
                    TailCall,

            // Get the Higher 16 bits from a 32-bit immediate
            // No relation with Alex Hi register
                    Hi,
            // Get the Lower 16 bits from a 32-bit immediate
            // No relation with Alex Lo register
                    Lo,

            // Handle gp_rel (small data/bss sections) relocation.
                    GPRel,

            // Thread Pointer
                    ThreadPointer,

            // Return
                    Ret,

            EH_RETURN,

            // DivRem(u)
                    DivRem,
            DivRemU,

            Wrapper,
            DynAlloc,

            Sync
        };
    }

    //===--------------------------------------------------------------------===//
    // TargetLowering Implementation
    //===--------------------------------------------------------------------===//
    class AlexFunctionInfo;
    class AlexSubtarget;
    class AlexTargetMachine;

    //@class AlexTargetLowering
    class AlexTargetLowering : public TargetLowering  {

    public:
        explicit AlexTargetLowering(const AlexTargetMachine *TM,
                                    const AlexSubtarget *STI,
                                    const AlexRegisterInfo*
        );

        /// getTargetNodeName - This method returns the name of a target specific
        //  DAG node.
        const char *getTargetNodeName(unsigned Opcode) const override;
    protected:
        const AlexSubtarget *subtarget;
    private:

//        // Create a TargetConstantPool node.
//        SDValue getTargetNode(ConstantPoolSDNode *N, EVT Ty, SelectionDAG &DAG,
//                              unsigned Flag) const;

        //- must be exist even without function all
        SDValue LowerFormalArguments(SDValue Chain,
                                     CallingConv::ID CallConv, bool IsVarArg,
                                     const SmallVectorImpl<ISD::InputArg> &Ins,
                                     SDLoc dl, SelectionDAG &DAG,
                                     SmallVectorImpl<SDValue> &InVals) const override;

        SDValue LowerReturn(SDValue Chain,
                            CallingConv::ID CallConv, bool IsVarArg,
                            const SmallVectorImpl<ISD::OutputArg> &Outs,
                            const SmallVectorImpl<SDValue> &OutVals,
                            SDLoc dl, SelectionDAG &DAG) const override;

    };
}

#endif // AlexISELLOWERING_H