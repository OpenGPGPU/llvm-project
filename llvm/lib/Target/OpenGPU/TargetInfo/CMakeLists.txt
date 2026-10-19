add_llvm_component_library(LLVMOpenGPUInfo
  OpenGPUTargetInfo.cpp

  LINK_COMPONENTS
  MC
  Support

  ADD_TO_COMPONENT
  OpenGPU
  )
