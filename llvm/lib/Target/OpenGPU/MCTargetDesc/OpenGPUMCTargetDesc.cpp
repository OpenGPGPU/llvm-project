//===-- OpenGPUMCTargetDesc.cpp - OpenGPU Target Descriptions -----------------===//
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

#include "OpenGPUMCTargetDesc.h"
#include "OpenGPUInstPrinter.h"
#include "OpenGPUMCAsmInfo.h"
#include "TargetInfo/OpenGPUTargetInfo.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrAnalysis.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/TargetParser/Triple.h"
#include <cstdint>
#include <string>

#define GET_INSTRINFO_MC_DESC
#define ENABLE_INSTR_PREDICATE_VERIFIER
#include "OpenGPUGenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "OpenGPUGenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "OpenGPUGenRegisterInfo.inc"

using namespace llvm;

static MCInstrInfo *createOpenGPUMCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitOpenGPUMCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createOpenGPUMCRegisterInfo(const Triple & /*TT*/) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitOpenGPUMCRegisterInfo(X, OpenGPU::RCA, 0, 0, OpenGPU::PC);
  return X;
}

static MCSubtargetInfo *
createOpenGPUMCSubtargetInfo(const Triple &TT, StringRef CPU, StringRef FS) {
  std::string CPUName = std::string(CPU);
  if (CPUName.empty())
    CPUName = "generic";

  return createOpenGPUMCSubtargetInfoImpl(TT, CPUName, /*TuneCPU*/ CPUName, FS);
}

static MCStreamer *createMCStreamer(const Triple &T, MCContext &Context,
                                    std::unique_ptr<MCAsmBackend> &&MAB,
                                    std::unique_ptr<MCObjectWriter> &&OW,
                                    std::unique_ptr<MCCodeEmitter> &&Emitter) {
  if (!T.isOSBinFormatELF())
    llvm_unreachable("OS not supported");

  return createELFStreamer(Context, std::move(MAB), std::move(OW),
                           std::move(Emitter));
}

static MCInstPrinter *createOpenGPUMCInstPrinter(const Triple & /*T*/,
                                               unsigned SyntaxVariant,
                                               const MCAsmInfo &MAI,
                                               const MCInstrInfo &MII,
                                               const MCRegisterInfo &MRI) {
  if (SyntaxVariant == 0)
    return new OpenGPUInstPrinter(MAI, MII, MRI);
  return nullptr;
}

static MCRelocationInfo *createOpenGPUElfRelocation(const Triple &TheTriple,
                                                  MCContext &Ctx) {
  return createMCRelocationInfo(TheTriple, Ctx);
}

namespace {

class OpenGPUMCInstrAnalysis : public MCInstrAnalysis {
public:
  explicit OpenGPUMCInstrAnalysis(const MCInstrInfo *Info)
      : MCInstrAnalysis(Info) {}

  bool evaluateBranch(const MCInst &Inst, uint64_t Addr, uint64_t Size,
                      uint64_t &Target) const override {
    if (Inst.getNumOperands() == 0)
      return false;
    if (!isConditionalBranch(Inst) && !isUnconditionalBranch(Inst) &&
        !isCall(Inst))
      return false;

    if (Info->get(Inst.getOpcode()).operands()[0].OperandType ==
        MCOI::OPERAND_PCREL) {
      int64_t Imm = Inst.getOperand(0).getImm();
      Target = Addr + Size + Imm;
      return true;
    } else {
      int64_t Imm = Inst.getOperand(0).getImm();

      // Skip case where immediate is 0 as that occurs in file that isn't linked
      // and the branch target inferred would be wrong.
      if (Imm == 0)
        return false;

      Target = Imm;
      return true;
    }
  }
};

} // end anonymous namespace

static MCInstrAnalysis *createOpenGPUInstrAnalysis(const MCInstrInfo *Info) {
  return new OpenGPUMCInstrAnalysis(Info);
}

extern "C" LLVM_ABI LLVM_EXTERNAL_VISIBILITY void
LLVMInitializeOpenGPUTargetMC() {
  // Register the MC asm info.
  RegisterMCAsmInfo<OpenGPUMCAsmInfo> X(getTheOpenGPUTarget());

  // Register the MC instruction info.
  TargetRegistry::RegisterMCInstrInfo(getTheOpenGPUTarget(),
                                      createOpenGPUMCInstrInfo);

  // Register the MC register info.
  TargetRegistry::RegisterMCRegInfo(getTheOpenGPUTarget(),
                                    createOpenGPUMCRegisterInfo);

  // Register the MC subtarget info.
  TargetRegistry::RegisterMCSubtargetInfo(getTheOpenGPUTarget(),
                                          createOpenGPUMCSubtargetInfo);

  // Register the MC code emitter
  TargetRegistry::RegisterMCCodeEmitter(getTheOpenGPUTarget(),
                                        createOpenGPUMCCodeEmitter);

  // Register the ASM Backend
  TargetRegistry::RegisterMCAsmBackend(getTheOpenGPUTarget(),
                                       createOpenGPUAsmBackend);

  // Register the MCInstPrinter.
  TargetRegistry::RegisterMCInstPrinter(getTheOpenGPUTarget(),
                                        createOpenGPUMCInstPrinter);

  // Register the ELF streamer.
  TargetRegistry::RegisterELFStreamer(getTheOpenGPUTarget(), createMCStreamer);

  // Register the MC relocation info.
  TargetRegistry::RegisterMCRelocationInfo(getTheOpenGPUTarget(),
                                           createOpenGPUElfRelocation);

  // Register the MC instruction analyzer.
  TargetRegistry::RegisterMCInstrAnalysis(getTheOpenGPUTarget(),
                                          createOpenGPUInstrAnalysis);
}
