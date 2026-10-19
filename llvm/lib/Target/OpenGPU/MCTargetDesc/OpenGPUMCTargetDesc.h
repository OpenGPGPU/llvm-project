//===-- OpenGPUMCTargetDesc.h - OpenGPU Target Descriptions ---------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file provides OpenGPU specific target descriptions.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_OPENGPU_MCTARGETDESC_OPENGPUMCTARGETDESC_H
#define LLVM_LIB_TARGET_OPENGPU_MCTARGETDESC_OPENGPUMCTARGETDESC_H

#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCTargetOptions.h"
#include "llvm/Support/DataTypes.h"

namespace llvm {
class MCAsmBackend;
class MCCodeEmitter;
class MCContext;
class MCInstrInfo;
class MCObjectTargetWriter;
class MCSubtargetInfo;
class Target;

MCCodeEmitter *createOpenGPUMCCodeEmitter(const MCInstrInfo &MCII,
                                        MCContext &Ctx);

MCAsmBackend *createOpenGPUAsmBackend(const Target &T, const MCSubtargetInfo &STI,
                                    const MCRegisterInfo &MRI,
                                    const MCTargetOptions &Options);

std::unique_ptr<MCObjectTargetWriter> createOpenGPUELFObjectWriter(uint8_t OSABI);
} // namespace llvm

// Defines symbolic names for OpenGPU registers.  This defines a mapping from
// register name to register number.
#define GET_REGINFO_ENUM
#include "OpenGPUGenRegisterInfo.inc"

// Defines symbolic names for the OpenGPU instructions.
#define GET_INSTRINFO_ENUM
#define GET_INSTRINFO_MC_HELPER_DECLS
#include "OpenGPUGenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "OpenGPUGenSubtargetInfo.inc"

#endif // LLVM_LIB_TARGET_OPENGPU_MCTARGETDESC_OPENGPUMCTARGETDESC_H
