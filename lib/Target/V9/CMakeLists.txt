set(LLVM_TARGET_DEFINITIONS V9.td)

tablegen(LLVM V9GenRegisterInfo.inc -gen-register-info)
tablegen(LLVM V9GenInstrInfo.inc -gen-instr-info)
tablegen(LLVM V9GenDisassemblerTables.inc -gen-disassembler)
tablegen(LLVM V9GenMCCodeEmitter.inc -gen-emitter)
tablegen(LLVM V9GenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM V9GenAsmMatcher.inc -gen-asm-matcher)
tablegen(LLVM V9GenDAGISel.inc -gen-dag-isel)
tablegen(LLVM V9GenSubtargetInfo.inc -gen-subtarget)
tablegen(LLVM V9GenCallingConv.inc -gen-callingconv)
add_public_tablegen_target(V9CommonTableGen)

add_llvm_target(V9CodeGen
  DelaySlotFiller.cpp
  V9AsmPrinter.cpp
  V9InstrInfo.cpp
  V9ISelDAGToDAG.cpp
  V9ISelLowering.cpp
  V9FrameLowering.cpp
  V9MachineFunctionInfo.cpp
  V9RegisterInfo.cpp
  V9Subtarget.cpp
  V9TargetMachine.cpp
  V9MCInstLower.cpp
  V9TargetObjectFile.cpp
  )

add_subdirectory(TargetInfo)
add_subdirectory(MCTargetDesc)
add_subdirectory(InstPrinter)
add_subdirectory(AsmParser)
add_subdirectory(Disassembler)
