set(LLVM_TARGET_DEFINITIONS V9Others.td)

# Generate Cpu0GenRegisterInfo.inc and Cpu0GenInstrInfo.inc which included by
#  your hand code C++ files.
# Cpu0GenRegisterInfo.inc came from Cpu0RegisterInfo.td, Cpu0GenInstrInfo.inc
#  came from Cpu0InstrInfo.td.
tablegen(LLVM V9GenRegisterInfo.inc -gen-register-info)
tablegen(LLVM V9GenInstrInfo.inc -gen-instr-info)
#tablegen(LLVM Cpu0GenSubtargetInfo.inc -gen-subtarget)
#tablegen(LLVM Cpu0GenMCPseudoLowering.inc -gen-pseudo-lowering)

# Cpu0CommonTableGen must be defined
add_public_tablegen_target(V9CommonTableGen)

# Cpu0CodeGen should match with LLVMBuild.txt Cpu0CodeGen
add_llvm_target(V9CodeGen
        V9TargetMachine.cpp
        )

# Should match with "subdirectories =  MCTargetDesc TargetInfo" in LLVMBuild.txt
add_subdirectory(TargetInfo)
add_subdirectory(MCTargetDesc)