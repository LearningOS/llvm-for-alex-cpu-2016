//===-- Cpu0ISelLowering.h - Cpu0 DAG Lowering Interface --------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the interfaces that Cpu0 uses to lower LLVM code into a
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

  protected:
    // Subtarget Info
    const V9CpuSubtarget &Subtarget;
    // Cache the ABI from the TargetMachine, we use it everywhere.
    const V9CpuABIInfo &ABI;

  private:

#if 0
    // Create a TargetConstantPool node.
    SDValue getTargetNode(ConstantPoolSDNode *N, EVT Ty, SelectionDAG &DAG,
                          unsigned Flag) const;
#endif

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
  const V9CpuTargetLowering *
  createV9CpuSETargetLowering(const V9CpuTargetMachine &TM, const V9CpuSubtarget &STI);
}

#endif // V9CpuISELLOWERING_H