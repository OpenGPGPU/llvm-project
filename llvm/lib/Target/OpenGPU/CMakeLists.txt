add_llvm_component_group(OpenGPU)

set(LLVM_TARGET_DEFINITIONS OpenGPU.td)

tablegen(LLVM OpenGPUGenAsmMatcher.inc -gen-asm-matcher)
tablegen(LLVM OpenGPUGenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM OpenGPUGenCallingConv.inc -gen-callingconv)
tablegen(LLVM OpenGPUGenDAGISel.inc -gen-dag-isel)
tablegen(LLVM OpenGPUGenDisassemblerTables.inc -gen-disassembler)
tablegen(LLVM OpenGPUGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM OpenGPUGenMCCodeEmitter.inc -gen-emitter)
tablegen(LLVM OpenGPUGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM OpenGPUGenSDNodeInfo.inc -gen-sd-node-info)
tablegen(LLVM OpenGPUGenSubtargetInfo.inc -gen-subtarget)

add_public_tablegen_target(OpenGPUCommonTableGen)

add_llvm_target(OpenGPUCodeGen
  OpenGPUAsmPrinter.cpp
  OpenGPUCodeGenPassBuilder.cpp
  OpenGPUDelaySlotFiller.cpp
  OpenGPUFrameLowering.cpp
  OpenGPUInstrInfo.cpp
  OpenGPUISelDAGToDAG.cpp
  OpenGPUISelLowering.cpp
  OpenGPUMachineFunctionInfo.cpp
  OpenGPUMCInstLower.cpp
  OpenGPUMemAluCombiner.cpp
  OpenGPURegisterInfo.cpp
  OpenGPUSelectionDAGInfo.cpp
  OpenGPUSubtarget.cpp
  OpenGPUTargetMachine.cpp
  OpenGPUTargetObjectFile.cpp

  LINK_COMPONENTS
  Analysis
  AsmPrinter
  CodeGen
  CodeGenTypes
  Core
  OpenGPUAsmParser
  OpenGPUDesc
  OpenGPUInfo
  MC
  Passes
  SelectionDAG
  Support
  Target
  TargetParser
  TransformUtils

  ADD_TO_COMPONENT
  OpenGPU
  )

add_subdirectory(AsmParser)
add_subdirectory(Disassembler)
add_subdirectory(MCTargetDesc)
add_subdirectory(TargetInfo)
