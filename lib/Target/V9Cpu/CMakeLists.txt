set(LLVM_TARGET_DEFINITIONS V9Cpu.td)

tablegen(LLVM V9CpuGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM V9CpuGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM V9CpuGenSubtargetInfo.inc -gen-subtarget)
tablegen(LLVM V9CpuGenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM V9CpuGenDAGISel.inc -gen-dag-isel)
tablegen(LLVM V9CpuGenCallingConv.inc -gen-callingconv)
tablegen(LLVM V9CpuGenCodeEmitter.inc -gen-emitter)
tablegen(LLVM V9CpuGenMCCodeEmitter.inc -gen-emitter)
add_public_tablegen_target(V9CpuCommonTableGen)

add_llvm_target(V9CpuCodeGen
        V9CpuAsmPrinter.cpp
        V9CpuMCInstLower.cpp
        V9CpuSEInstrInfo.cpp
        V9CpuSEISelLowering.cpp
        V9CpuSERegisterInfo.cpp
        V9CpuInstrInfo.cpp
        V9CpuISelLowering.cpp
        V9CpuSubtarget.cpp
        V9CpuSEFrameLowering.cpp
        V9CpuFrameLowering.cpp
        V9CpuRegisterInfo.cpp
        V9CpuTargetMachine.cpp
        V9CpuTargetObjectFile.cpp
        V9CpuMachineFunction.cpp
        V9CpuISelDAGToDAG.cpp
        V9CpuSEISelDAGToDAG.cpp
        V9CpuAnalyzeImmediate.cpp
        )

add_subdirectory(TargetInfo)
add_subdirectory(MCTargetDesc)
add_subdirectory(InstPrinter)
