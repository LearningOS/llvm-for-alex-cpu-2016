//===-- V9CpuISelLowering.h - V9Cpu DAG Lowering Interface --------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the interfaces that V9Cpu uses to lower LLVM code into a
// selection DAG.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_V9CPU_V9CPUISELLOWERING_H
#define LLVM_LIB_TARGET_V9CPU_V9CPUISELLOWERING_H


#include "MCTargetDesc/V9CpuABIInfo.h"
#include "V9Cpu.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/IR/Function.h"
#include "llvm/Target/TargetLowering.h"
#include <deque>

namespace llvm {
  namespace V9CpuISD {
    enum NodeType {
      // Start the numbering from where ISD NodeType finishes.
      FIRST_NUMBER = ISD::BUILTIN_OP_END,

      // Jump and link (call)
      JmpLink,

      // Tail call
      TailCall,

      // Get the Higher 16 bits from a 32-bit immediate
      // No relation with V9Cpu Hi register
      Hi,
      // Get the Lower 16 bits from a 32-bit immediate
      // No relation with V9Cpu Lo register
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
  class V9CpuFunctionInfo;
  class V9CpuSubtarget;

  //@class V9CpuTargetLowering
  class V9CpuTargetLowering : public TargetLowering  {

  public:
    explicit V9CpuTargetLowering(const V9CpuTargetMachine &TM,
                                const V9CpuSubtarget &STI);

    static const V9CpuTargetLowering *create(const V9CpuTargetMachine &TM,
                                            const V9CpuSubtarget &STI);

    /// getTargetNodeName - This method returns the name of a target specific
    //  DAG node.
    const char *getTargetNodeName(unsigned Opcode) const override;

  protected:

    /// ByValArgInfo - Byval argument information.
    struct ByValArgInfo {
      unsigned FirstIdx; // Index of the first register used.
      unsigned NumRegs;  // Number of registers used for this argument.
      unsigned Address;  // Offset of the stack area used to pass this argument.

      ByValArgInfo() : FirstIdx(0), NumRegs(0), Address(0) {}
    };
      class V9CpuCC {
      public:
          enum SpecialCallingConvType {
              NoSpecialCallingConv
          };

          V9CpuCC(CallingConv::ID CallConv, bool IsO32, CCState &Info,
                  SpecialCallingConvType SpecialCallingConv = NoSpecialCallingConv);

          void analyzeCallResult(const SmallVectorImpl<ISD::InputArg> &Ins,
                                 bool IsSoftFloat, const SDNode *CallNode,
                                 const Type *RetTy) const;

          void analyzeReturn(const SmallVectorImpl<ISD::OutputArg> &Outs,
                             bool IsSoftFloat, const Type *RetTy) const;

          const CCState &getCCInfo() const { return CCInfo; }

          /// hasByValArg - Returns true if function has byval arguments.
          bool hasByValArg() const { return !ByValArgs.empty(); }

          /// reservedArgArea - The size of the area the caller reserves for
          /// register arguments. This is 16-byte if ABI is O32.
          unsigned reservedArgArea() const {
              return (CallConv != CallingConv::Fast) ? 8 : 0;
          }

          typedef SmallVectorImpl<ByValArgInfo>::const_iterator byval_iterator;
          byval_iterator byval_begin() const { return ByValArgs.begin(); }
          byval_iterator byval_end() const { return ByValArgs.end(); }

      private:

          /// Return the type of the register which is used to pass an argument or
          /// return a value. This function returns f64 if the argument is an i64
          /// value which has been generated as a result of softening an f128 value.
          /// Otherwise, it just returns VT.
          MVT getRegVT(MVT VT, const Type *OrigTy, const SDNode *CallNode,
                       bool IsSoftFloat) const {
              return VT;
          }

          template<typename Ty>
          void analyzeReturn(const SmallVectorImpl<Ty> &RetVals, bool IsSoftFloat,
                             const SDNode *CallNode, const Type *RetTy) const;

          CCState &CCInfo;
          CallingConv::ID CallConv;
          bool IsO32;
          SmallVector<ByValArgInfo, 2> ByValArgs;
      };
  protected:
    // Subtarget Info
    const V9CpuSubtarget &Subtarget;
    // Cache the ABI from the TargetMachine, we use it everywhere.
    const V9CpuABIInfo &ABI;

  private:

    // Create a TargetConstantPool node.
    SDValue getTargetNode(ConstantPoolSDNode *N, EVT Ty, SelectionDAG &DAG,
                          unsigned Flag) const;

    // Lower Operand specifics
    SDValue lowerGlobalAddress(SDValue Op, SelectionDAG &DAG) const;

	//- must be exist even without function all
    SDValue
      LowerFormalArguments(SDValue Chain,
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

#endif // V9CpuISELLOWERING_H