//===-- RISCVTargetInfo.cpp - RISC-V Target Implementation ----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "TargetInfo/RISCVTargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Compiler.h"
using namespace llvm;

Target &llvm::getTheRISCV32Target() {
  static Target TheRISCV32Target;
  return TheRISCV32Target;
}

Target &llvm::getTheRISCV64Target() {
  static Target TheRISCV64Target;
  return TheRISCV64Target;
}

Target &llvm::getTheRISCV32beTarget() {
  static Target TheRISCV32beTarget;
  return TheRISCV32beTarget;
}

Target &llvm::getTheRISCV64beTarget() {
  static Target TheRISCV64beTarget;
  return TheRISCV64beTarget;
}

Target &llvm::getTheOpenGPUTarget() {
  static Target TheOpenGPUTarget;
  return TheOpenGPUTarget;
}

extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void
LLVMInitializeRISCVTargetInfo() {
  RegisterTarget<Triple::riscv32, /*HasJIT=*/true> X(
      getTheRISCV32Target(), "riscv32", "32-bit RISC-V", "RISCV");
  RegisterTarget<Triple::riscv64, /*HasJIT=*/true> Y(
      getTheRISCV64Target(), "riscv64", "64-bit RISC-V", "RISCV");
  RegisterTarget<Triple::riscv32be> A(getTheRISCV32beTarget(), "riscv32be",
                                      "32-bit big endian RISC-V", "RISCV");
  RegisterTarget<Triple::riscv64be> B(getTheRISCV64beTarget(), "riscv64be",
                                      "64-bit big endian RISC-V", "RISCV");
  // OpenGPU is a 32-bit RISC-V SIMT GPU.  It reuses the RISCV backend and MC
  // layers; only the GPU custom instructions (the '+xopengpu' feature) are
  // OpenGPU-specific.
  RegisterTarget<Triple::opengpu, /*HasJIT=*/true> C(
      getTheOpenGPUTarget(), "opengpu", "OpenGPU SIMT GPU", "RISCV");
}
