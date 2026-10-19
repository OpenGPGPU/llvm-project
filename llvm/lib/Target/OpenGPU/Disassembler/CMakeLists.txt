add_llvm_component_library(LLVMOpenGPUDisassembler
  OpenGPUDisassembler.cpp

  LINK_COMPONENTS
  CodeGenTypes
  OpenGPUDesc
  OpenGPUInfo
  MC
  MCDisassembler
  Support

  ADD_TO_COMPONENT
  OpenGPU
  )
