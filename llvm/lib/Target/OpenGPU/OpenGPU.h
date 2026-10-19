//===-- OpenGPU.h - Top-level interface for OpenGPU representation --*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the entry points for global functions defined in the LLVM
// OpenGPU back-end.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_OPENGPU_OPENGPU_H
#define LLVM_LIB_TARGET_OPENGPU_OPENGPU_H

#include "llvm/CodeGen/MachineFunctionAnalysisManager.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/IR/Analysis.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

namespace llvm {
class FunctionPass;
class OpenGPUTargetMachine;
class PassRegistry;

// createOpenGPUISelDagPass - This pass converts a legalized DAG into a
// OpenGPU-specific DAG, ready for instruction scheduling.
class OpenGPUISelDAGToDAGPass : public SelectionDAGISelPass {
public:
  OpenGPUISelDAGToDAGPass(OpenGPUTargetMachine &TM);
};

FunctionPass *createOpenGPUISelDagLegacyPass(OpenGPUTargetMachine &TM);

// createOpenGPUDelaySlotFillerPass - This pass fills delay slots
// with useful instructions or nop's
class OpenGPUDelaySlotFillerPass
    : public RequiredPassInfoMixin<OpenGPUDelaySlotFillerPass> {
public:
  PreservedAnalyses run(MachineFunction &MF,
                        MachineFunctionAnalysisManager &MFAM);
};

FunctionPass *
createOpenGPUDelaySlotFillerLegacyPass(const OpenGPUTargetMachine &TM);

// createOpenGPUMemAluCombinerPass - This pass combines loads/stores and
// arithmetic operations.
class OpenGPUMemAluCombinerPass
    : public RequiredPassInfoMixin<OpenGPUMemAluCombinerPass> {
public:
  PreservedAnalyses run(MachineFunction &MF,
                        MachineFunctionAnalysisManager &MFAM);
};

FunctionPass *createOpenGPUMemAluCombinerLegacyPass();

void initializeOpenGPUAsmPrinterPass(PassRegistry &);
void initializeOpenGPUDAGToDAGISelLegacyPass(PassRegistry &);
void initializeOpenGPUMemAluCombinerLegacyPass(PassRegistry &);

} // namespace llvm

#endif // LLVM_LIB_TARGET_OPENGPU_OPENGPU_H
