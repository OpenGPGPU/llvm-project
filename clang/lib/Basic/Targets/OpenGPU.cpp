//===--- OpenGPU.cpp - Implement OpenGPU target feature support -----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements OpenGPUTargetInfo, a RISCV32 derivative for the OpenGPU
// SIMT GPU.
//
//===----------------------------------------------------------------------===//

#include "OpenGPU.h"
#include "clang/Basic/MacroBuilder.h"

using namespace clang;
using namespace clang::targets;

void OpenGPUTargetInfo::getTargetDefines(const LangOptions &Opts,
                                         MacroBuilder &Builder) const {
  RISCV32TargetInfo::getTargetDefines(Opts, Builder);
  Builder.defineMacro("__opengpu__");
  Builder.defineMacro("__OPENGPU__");
}

bool OpenGPUTargetInfo::hasFeature(StringRef Feature) const {
  if (Feature == "opengpu")
    return true;
  return RISCV32TargetInfo::hasFeature(Feature);
}
