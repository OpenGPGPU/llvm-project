//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_OPENGPU_OPENGPUASMPRINTER_H
#define LLVM_LIB_TARGET_OPENGPU_OPENGPUASMPRINTER_H

#include "llvm/CodeGen/MachineFunctionAnalysisManager.h"
#include "llvm/IR/Analysis.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"

namespace llvm {

class OpenGPUAsmPrinterBeginPass
    : public RequiredPassInfoMixin<OpenGPUAsmPrinterBeginPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &MFAM);
};

class OpenGPUAsmPrinterPass : public RequiredPassInfoMixin<OpenGPUAsmPrinterPass> {
public:
  PreservedAnalyses run(MachineFunction &MF,
                        MachineFunctionAnalysisManager &MFAM);
};

class OpenGPUAsmPrinterEndPass
    : public RequiredPassInfoMixin<OpenGPUAsmPrinterEndPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &MFAM);
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_OPENGPU_OPENGPUASMPRINTER_H
