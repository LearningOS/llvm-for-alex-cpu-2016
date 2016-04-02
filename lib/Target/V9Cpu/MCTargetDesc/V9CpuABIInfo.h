#ifndef LLVM_LIB_TARGET_V9CPU_MCTARGETDESC_V9CPUABIINFO_H
#define LLVM_LIB_TARGET_V9CPU_MCTARGETDESC_V9CPUABIINFO_H


#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/Triple.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/MC/MCRegisterInfo.h"

namespace llvm {

class MCTargetOptions;
class StringRef;
class TargetRegisterClass;

class V9CpuABIInfo {
public:
  enum class ABI { Unknown, O32, S32 };

protected:
  ABI ThisABI;

public:
    V9CpuABIInfo(ABI ThisABI) : ThisABI(ThisABI) {}

  static V9CpuABIInfo Unknown() { return V9CpuABIInfo(ABI::Unknown); }
  static V9CpuABIInfo O32() { return V9CpuABIInfo(ABI::O32); }
  static V9CpuABIInfo S32() { return V9CpuABIInfo(ABI::S32); }
  static V9CpuABIInfo computeTargetABI();

  bool IsKnown() const { return ThisABI != ABI::Unknown; }
  bool IsO32() const { return ThisABI == ABI::O32; }
  bool IsS32() const { return ThisABI == ABI::S32; }
  ABI GetEnumValue() const { return ThisABI; }

  /// The registers to use for byval arguments.
  const ArrayRef<MCPhysReg> GetByValArgRegs() const;

  /// The registers to use for the variable argument list.
  const ArrayRef<MCPhysReg> GetVarArgRegs() const;

  /// Obtain the size of the area allocated by the callee for arguments.
  /// CallingConv::FastCall affects the value for O32.
  unsigned GetCalleeAllocdArgSizeInBytes(CallingConv::ID CC) const;

  /// Ordering of ABI's
  /// V9CpuGenSubtargetInfo.inc will use this to resolve conflicts when given
  /// multiple ABI options.
  bool operator<(const V9CpuABIInfo Other) const {
    return ThisABI < Other.GetEnumValue();
  }

  unsigned GetStackPtr() const;
  unsigned GetFramePtr() const;
  unsigned GetNullPtr() const;

  unsigned GetEhDataReg(unsigned I) const;
  int EhDataRegSize() const;
};
}

#endif