//===-- OpenGPUELFObjectWriter.cpp - OpenGPU ELF Writer -----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/OpenGPUBaseInfo.h"
#include "MCTargetDesc/OpenGPUFixupKinds.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

namespace {

class OpenGPUELFObjectWriter : public MCELFObjectTargetWriter {
public:
  explicit OpenGPUELFObjectWriter(uint8_t OSABI);

  ~OpenGPUELFObjectWriter() override = default;

protected:
  unsigned getRelocType(const MCFixup &, const MCValue &,
                        bool IsPCRel) const override;
  bool needsRelocateWithSymbol(const MCValue &, unsigned Type) const override;
};

} // end anonymous namespace

OpenGPUELFObjectWriter::OpenGPUELFObjectWriter(uint8_t OSABI)
    : MCELFObjectTargetWriter(/*Is64Bit_=*/false, OSABI, ELF::EM_OPENGPU,
                              /*HasRelocationAddend_=*/true) {}

unsigned OpenGPUELFObjectWriter::getRelocType(const MCFixup &Fixup,
                                            const MCValue &, bool) const {
  unsigned Type;
  unsigned Kind = static_cast<unsigned>(Fixup.getKind());
  switch (Kind) {
  case OpenGPU::FIXUP_OPENGPU_21:
    Type = ELF::R_OPENGPU_21;
    break;
  case OpenGPU::FIXUP_OPENGPU_21_F:
    Type = ELF::R_OPENGPU_21_F;
    break;
  case OpenGPU::FIXUP_OPENGPU_25:
    Type = ELF::R_OPENGPU_25;
    break;
  case OpenGPU::FIXUP_OPENGPU_32:
  case FK_Data_4:
    Type = ELF::R_OPENGPU_32;
    break;
  case OpenGPU::FIXUP_OPENGPU_HI16:
    Type = ELF::R_OPENGPU_HI16;
    break;
  case OpenGPU::FIXUP_OPENGPU_LO16:
    Type = ELF::R_OPENGPU_LO16;
    break;
  case OpenGPU::FIXUP_OPENGPU_NONE:
    Type = ELF::R_OPENGPU_NONE;
    break;

  default:
    llvm_unreachable("Invalid fixup kind!");
  }
  return Type;
}

bool OpenGPUELFObjectWriter::needsRelocateWithSymbol(const MCValue &,
                                                   unsigned Type) const {
  switch (Type) {
  case ELF::R_OPENGPU_21:
  case ELF::R_OPENGPU_21_F:
  case ELF::R_OPENGPU_25:
  case ELF::R_OPENGPU_32:
  case ELF::R_OPENGPU_HI16:
    return true;
  default:
    return false;
  }
}

std::unique_ptr<MCObjectTargetWriter>
llvm::createOpenGPUELFObjectWriter(uint8_t OSABI) {
  return std::make_unique<OpenGPUELFObjectWriter>(OSABI);
}
