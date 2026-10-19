//===--- OpenGPU.h - Declare OpenGPU target feature support ----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares OpenGPU TargetInfo objects.  OpenGPU is a RISCV vendor
// target: it reuses the RISCV32 frontend, data layout and ABI, and adds the
// XOpengpu custom instructions on top of RV32IMAFV.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_BASIC_TARGETS_OPENGPU_H
#define LLVM_CLANG_LIB_BASIC_TARGETS_OPENGPU_H

#include "RISCV.h"

namespace clang {
namespace targets {

class LLVM_LIBRARY_VISIBILITY OpenGPUTargetInfo : public RISCV32TargetInfo {
public:
  OpenGPUTargetInfo(const llvm::Triple &Triple, const TargetOptions &Opts)
      : RISCV32TargetInfo(Triple, Opts) {
    CPU = "opengpu";
    ABI = "ilp32";
  }

  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;

  bool hasFeature(StringRef Feature) const override;
};

} // namespace targets
} // namespace clang

#endif // LLVM_CLANG_LIB_BASIC_TARGETS_OPENGPU_H
