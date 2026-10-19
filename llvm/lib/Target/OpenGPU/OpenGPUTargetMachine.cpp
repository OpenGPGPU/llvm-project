//===-- OpenGPUTargetMachine.cpp - Define TargetMachine for OpenGPU ---------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Implements the info about OpenGPU target spec.
//
//===----------------------------------------------------------------------===//

#include "OpenGPUTargetMachine.h"

#include "OpenGPU.h"
#include "OpenGPUMachineFunctionInfo.h"
#include "OpenGPUTargetObjectFile.h"
#include "OpenGPUTargetTransformInfo.h"
#include "TargetInfo/OpenGPUTargetInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Target/TargetOptions.h"
#include <optional>

using namespace llvm;

extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void LLVMInitializeOpenGPUTarget() {
  // Register the target.
  RegisterTargetMachine<OpenGPUTargetMachine> registered_target(
      getTheOpenGPUTarget());
  PassRegistry &PR = *PassRegistry::getPassRegistry();
  initializeOpenGPUAsmPrinterPass(PR);
  initializeOpenGPUDAGToDAGISelLegacyPass(PR);
  initializeOpenGPUMemAluCombinerLegacyPass(PR);
}

static Reloc::Model getEffectiveRelocModel(std::optional<Reloc::Model> RM) {
  return RM.value_or(Reloc::PIC_);
}

OpenGPUTargetMachine::OpenGPUTargetMachine(
    const Target &T, const Triple &TT, StringRef Cpu, StringRef FeatureString,
    const TargetOptions &Options, std::optional<Reloc::Model> RM,
    std::optional<CodeModel::Model> CodeModel, CodeGenOptLevel OptLevel,
    bool JIT)
    : CodeGenTargetMachineImpl(
          T, TT, Cpu, FeatureString, Options, getEffectiveRelocModel(RM),
          getEffectiveCodeModel(CodeModel, CodeModel::Medium), OptLevel),
      Subtarget(TT, Cpu, FeatureString, *this, Options, getCodeModel(),
                OptLevel),
      TLOF(new OpenGPUTargetObjectFile()) {
  initAsmInfo();
}

TargetTransformInfo
OpenGPUTargetMachine::getTargetTransformInfo(const Function &F) const {
  return TargetTransformInfo(std::make_unique<OpenGPUTTIImpl>(this, F));
}

MachineFunctionInfo *OpenGPUTargetMachine::createMachineFunctionInfo(
    BumpPtrAllocator &Allocator, const Function &F,
    const TargetSubtargetInfo *STI) const {
  return OpenGPUMachineFunctionInfo::create<OpenGPUMachineFunctionInfo>(Allocator,
                                                                    F, STI);
}

namespace {
// OpenGPU Code Generator Pass Configuration Options.
class OpenGPUPassConfig : public TargetPassConfig {
public:
  OpenGPUPassConfig(OpenGPUTargetMachine &TM, PassManagerBase *PassManager)
      : TargetPassConfig(TM, *PassManager) {}

  OpenGPUTargetMachine &getOpenGPUTargetMachine() const {
    return getTM<OpenGPUTargetMachine>();
  }

  void addIRPasses() override;
  bool addInstSelector() override;
  void addPreSched2() override;
  void addPreEmitPass() override;
};
} // namespace

TargetPassConfig *
OpenGPUTargetMachine::createPassConfig(PassManagerBase &PassManager) {
  return new OpenGPUPassConfig(*this, &PassManager);
}

void OpenGPUPassConfig::addIRPasses() {
  addPass(createAtomicExpandLegacyPass());

  TargetPassConfig::addIRPasses();
}

// Install an instruction selector pass.
bool OpenGPUPassConfig::addInstSelector() {
  addPass(createOpenGPUISelDagLegacyPass(getOpenGPUTargetMachine()));
  return false;
}

// Implemented by targets that want to run passes immediately before
// machine code is emitted.
void OpenGPUPassConfig::addPreEmitPass() {
  addPass(createOpenGPUDelaySlotFillerLegacyPass(getOpenGPUTargetMachine()));
}

// Run passes after prolog-epilog insertion and before the second instruction
// scheduling pass.
void OpenGPUPassConfig::addPreSched2() {
  addPass(createOpenGPUMemAluCombinerLegacyPass());
}
