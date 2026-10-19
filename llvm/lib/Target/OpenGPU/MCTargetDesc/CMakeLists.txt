add_llvm_component_library(LLVMOpenGPUDesc
  OpenGPUAsmBackend.cpp
  OpenGPUELFObjectWriter.cpp
  OpenGPUInstPrinter.cpp
  OpenGPUMCAsmInfo.cpp
  OpenGPUMCCodeEmitter.cpp
  OpenGPUMCTargetDesc.cpp

  LINK_COMPONENTS
  OpenGPUInfo
  MC
  MCDisassembler
  Support
  TargetParser

  ADD_TO_COMPONENT
  OpenGPU
)
