//===-- OpenGPUBaseInfo.h - Top level definitions for OpenGPU MC ----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains small standalone helper functions and enum definitions for
// the OpenGPU target useful for the compiler back-end and the MC libraries.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_OPENGPU_MCTARGETDESC_OPENGPUBASEINFO_H
#define LLVM_LIB_TARGET_OPENGPU_MCTARGETDESC_OPENGPUBASEINFO_H

#include "OpenGPUMCTargetDesc.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/Support/DataTypes.h"
#include "llvm/Support/ErrorHandling.h"

namespace llvm {

// OpenGPUII - This namespace holds all of the target specific flags that
// instruction info tracks.
namespace OpenGPUII {
// Target Operand Flag enum.
enum TOF {
  //===------------------------------------------------------------------===//
  // OpenGPU Specific MachineOperand flags.
  MO_NO_FLAG,

  // MO_ABS_HI/LO - Represents the hi or low part of an absolute symbol
  // address.
  MO_ABS_HI,
  MO_ABS_LO,
};
} // namespace OpenGPUII

static inline unsigned getOpenGPURegisterNumbering(MCRegister Reg) {
  switch (Reg.id()) {
  case OpenGPU::R0:
    return 0;
  case OpenGPU::R1:
    return 1;
  case OpenGPU::R2:
  case OpenGPU::PC:
    return 2;
  case OpenGPU::R3:
    return 3;
  case OpenGPU::R4:
  case OpenGPU::SP:
    return 4;
  case OpenGPU::R5:
  case OpenGPU::FP:
    return 5;
  case OpenGPU::R6:
    return 6;
  case OpenGPU::R7:
    return 7;
  case OpenGPU::R8:
  case OpenGPU::RV:
    return 8;
  case OpenGPU::R9:
    return 9;
  case OpenGPU::R10:
  case OpenGPU::RR1:
    return 10;
  case OpenGPU::R11:
  case OpenGPU::RR2:
    return 11;
  case OpenGPU::R12:
    return 12;
  case OpenGPU::R13:
    return 13;
  case OpenGPU::R14:
    return 14;
  case OpenGPU::R15:
  case OpenGPU::RCA:
    return 15;
  case OpenGPU::R16:
    return 16;
  case OpenGPU::R17:
    return 17;
  case OpenGPU::R18:
    return 18;
  case OpenGPU::R19:
    return 19;
  case OpenGPU::R20:
    return 20;
  case OpenGPU::R21:
    return 21;
  case OpenGPU::R22:
    return 22;
  case OpenGPU::R23:
    return 23;
  case OpenGPU::R24:
    return 24;
  case OpenGPU::R25:
    return 25;
  case OpenGPU::R26:
    return 26;
  case OpenGPU::R27:
    return 27;
  case OpenGPU::R28:
    return 28;
  case OpenGPU::R29:
    return 29;
  case OpenGPU::R30:
    return 30;
  case OpenGPU::R31:
    return 31;
  default:
    llvm_unreachable("Unknown register number!");
  }
}
} // namespace llvm
#endif // LLVM_LIB_TARGET_OPENGPU_MCTARGETDESC_OPENGPUBASEINFO_H
