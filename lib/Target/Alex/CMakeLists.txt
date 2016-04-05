set(LLVM_TARGET_DEFINITIONS Alex.td)

tablegen(LLVM AlexGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM AlexGenInstrInfo.inc -gen-instr-info)
#tablegen(LLVM AlexGenDisassemblerTables.inc -gen-disassembler)
#tablegen(LLVM AlexGenMCCodeEmitter.inc -gen-emitter)
tablegen(LLVM AlexGenAsmWriter.inc -gen-asm-writer)
#tablegen(LLVM AlexGenAsmMatcher.inc -gen-asm-matcher)
tablegen(LLVM AlexGenDAGISel.inc -gen-dag-isel)
tablegen(LLVM AlexGenSubtargetInfo.inc -gen-subtarget)
tablegen(LLVM AlexGenCallingConv.inc -gen-callingconv)
add_public_tablegen_target(AlexCommonTableGen)

add_llvm_target(AlexCodeGen
        AlexAsmPrinter.cpp
        AlexInstrInfo.cpp
        AlexISelDAGToDAG.cpp
        AlexISelLowering.cpp
        AlexFrameLowering.cpp
#        AlexMachineFunctionInfo.cpp
        AlexRegisterInfo.cpp
        AlexTargetMachine.cpp
#        AlexMCInstLower.cpp
        AlexTargetObjectFile.cpp
        )

add_subdirectory(MCTargetDesc)
add_subdirectory(InstPrinter)
