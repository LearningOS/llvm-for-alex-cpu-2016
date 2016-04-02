#ifndef LLVM_LIB_TARGET_V9CPU_MCTARGETDESC_V9CPUMCTARGETDESC_H
#define LLVM_LIB_TARGET_V9CPU_MCTARGETDESC_V9CPUMCTARGETDESC_H

#include "llvm/Support/DataTypes.h"

namespace llvm {
    class Target;
    class Triple;

    extern Target TheV9CpuTarget;

} // End llvm namespace

// Defines symbolic names for Cpu0 registers.  This defines a mapping from
// register name to register number.
#define GET_REGINFO_ENUM
#include "V9CpuGenRegisterInfo.inc"

// Defines symbolic names for the Cpu0 instructions.
#define GET_INSTRINFO_ENUM
#include "V9CpuGenInstrInfo.inc"

#endif