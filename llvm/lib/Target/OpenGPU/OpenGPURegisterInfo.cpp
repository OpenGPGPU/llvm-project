//===-- OpenGPURegisterInfo.cpp - OpenGPU Register Information ------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the OpenGPU implementation of the TargetRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#include "OpenGPURegisterInfo.h"
#include "OpenGPUAluCode.h"
#include "OpenGPUCondCode.h"
#include "OpenGPUFrameLowering.h"
#include "OpenGPUInstrInfo.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/RegisterScavenging.h"
#include "llvm/CodeGen/TargetFrameLowering.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/Support/ErrorHandling.h"

#define GET_REGINFO_TARGET_DESC
#include "OpenGPUGenRegisterInfo.inc"

using namespace llvm;

OpenGPURegisterInfo::OpenGPURegisterInfo() : OpenGPUGenRegisterInfo(OpenGPU::RCA) {}

const uint16_t *
OpenGPURegisterInfo::getCalleeSavedRegs(const MachineFunction * /*MF*/) const {
  return CSR_SaveList;
}

BitVector OpenGPURegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());

  Reserved.set(OpenGPU::R0);
  Reserved.set(OpenGPU::R1);
  Reserved.set(OpenGPU::PC);
  Reserved.set(OpenGPU::R2);
  Reserved.set(OpenGPU::SP);
  Reserved.set(OpenGPU::R4);
  Reserved.set(OpenGPU::FP);
  Reserved.set(OpenGPU::R5);
  Reserved.set(OpenGPU::RR1);
  Reserved.set(OpenGPU::R10);
  Reserved.set(OpenGPU::RR2);
  Reserved.set(OpenGPU::R11);
  Reserved.set(OpenGPU::RCA);
  Reserved.set(OpenGPU::R15);
  if (hasBasePointer(MF))
    Reserved.set(getBaseRegister());
  return Reserved;
}

bool OpenGPURegisterInfo::requiresRegisterScavenging(
    const MachineFunction & /*MF*/) const {
  return true;
}

static bool isALUArithLoOpcode(unsigned Opcode) {
  switch (Opcode) {
  case OpenGPU::ADD_I_LO:
  case OpenGPU::SUB_I_LO:
  case OpenGPU::ADD_F_I_LO:
  case OpenGPU::SUB_F_I_LO:
  case OpenGPU::ADDC_I_LO:
  case OpenGPU::SUBB_I_LO:
  case OpenGPU::ADDC_F_I_LO:
  case OpenGPU::SUBB_F_I_LO:
    return true;
  default:
    return false;
  }
}

static unsigned getOppositeALULoOpcode(unsigned Opcode) {
  switch (Opcode) {
  case OpenGPU::ADD_I_LO:
    return OpenGPU::SUB_I_LO;
  case OpenGPU::SUB_I_LO:
    return OpenGPU::ADD_I_LO;
  case OpenGPU::ADD_F_I_LO:
    return OpenGPU::SUB_F_I_LO;
  case OpenGPU::SUB_F_I_LO:
    return OpenGPU::ADD_F_I_LO;
  case OpenGPU::ADDC_I_LO:
    return OpenGPU::SUBB_I_LO;
  case OpenGPU::SUBB_I_LO:
    return OpenGPU::ADDC_I_LO;
  case OpenGPU::ADDC_F_I_LO:
    return OpenGPU::SUBB_F_I_LO;
  case OpenGPU::SUBB_F_I_LO:
    return OpenGPU::ADDC_F_I_LO;
  default:
    llvm_unreachable("Invalid ALU lo opcode");
  }
}

static unsigned getRRMOpcodeVariant(unsigned Opcode) {
  switch (Opcode) {
  case OpenGPU::LDBs_RI:
    return OpenGPU::LDBs_RR;
  case OpenGPU::LDBz_RI:
    return OpenGPU::LDBz_RR;
  case OpenGPU::LDHs_RI:
    return OpenGPU::LDHs_RR;
  case OpenGPU::LDHz_RI:
    return OpenGPU::LDHz_RR;
  case OpenGPU::LDW_RI:
    return OpenGPU::LDW_RR;
  case OpenGPU::STB_RI:
    return OpenGPU::STB_RR;
  case OpenGPU::STH_RI:
    return OpenGPU::STH_RR;
  case OpenGPU::SW_RI:
    return OpenGPU::SW_RR;
  default:
    llvm_unreachable("Opcode has no RRM variant");
  }
}

bool OpenGPURegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                            int SPAdj, unsigned FIOperandNum,
                                            RegScavenger *RS) const {
  assert(SPAdj == 0 && "Unexpected");

  MachineInstr &MI = *II;
  MachineFunction &MF = *MI.getParent()->getParent();
  const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();
  const TargetFrameLowering *TFI = MF.getSubtarget().getFrameLowering();
  bool HasFP = TFI->hasFP(MF);
  DebugLoc DL = MI.getDebugLoc();

  int FrameIndex = MI.getOperand(FIOperandNum).getIndex();

  int Offset = MF.getFrameInfo().getObjectOffset(FrameIndex) +
               MI.getOperand(FIOperandNum + 1).getImm();

  // Addressable stack objects are addressed using neg. offsets from fp
  // or pos. offsets from sp/basepointer
  if (!HasFP || (hasStackRealignment(MF) && FrameIndex >= 0))
    Offset += MF.getFrameInfo().getStackSize();

  Register FrameReg = getFrameRegister(MF);
  if (FrameIndex >= 0) {
    if (hasBasePointer(MF))
      FrameReg = getBaseRegister();
    else if (hasStackRealignment(MF))
      FrameReg = OpenGPU::SP;
  }

  // Replace frame index with a frame pointer reference.
  // If the offset is small enough to fit in the immediate field, directly
  // encode it.
  // Otherwise scavenge a register and encode it into a MOVHI, OR_I_LO sequence.
  if ((isSPLSOpcode(MI.getOpcode()) && !isInt<10>(Offset)) ||
      !isInt<16>(Offset)) {
    assert(RS && "Register scavenging must be on");
    Register Reg = RS->FindUnusedReg(&OpenGPU::GPRRegClass);
    if (!Reg)
      Reg = RS->scavengeRegisterBackwards(OpenGPU::GPRRegClass, II, false, SPAdj);
    assert(Reg && "Register scavenger failed");

    bool HasNegOffset = false;
    // ALU ops have unsigned immediate values. If the Offset is negative, we
    // negate it here and reverse the opcode later.
    if (Offset < 0) {
      HasNegOffset = true;
      Offset = -Offset;
    }

    if (!isInt<16>(Offset)) {
      // Reg = hi(offset) | lo(offset)
      BuildMI(*MI.getParent(), II, DL, TII->get(OpenGPU::MOVHI), Reg)
          .addImm(static_cast<uint32_t>(Offset) >> 16);
      BuildMI(*MI.getParent(), II, DL, TII->get(OpenGPU::OR_I_LO), Reg)
          .addReg(Reg)
          .addImm(Offset & 0xffffU);
    } else {
      // Reg = mov(offset)
      BuildMI(*MI.getParent(), II, DL, TII->get(OpenGPU::ADD_I_LO), Reg)
          .addImm(0)
          .addImm(Offset);
    }
    // Reg = FrameReg OP Reg
    if (MI.getOpcode() == OpenGPU::ADD_I_LO) {
      BuildMI(*MI.getParent(), II, DL,
              HasNegOffset ? TII->get(OpenGPU::SUB_R) : TII->get(OpenGPU::ADD_R),
              MI.getOperand(0).getReg())
          .addReg(FrameReg)
          .addReg(Reg)
          .addImm(LPCC::ICC_T);
      MI.eraseFromParent();
      return true;
    }
    if (isSPLSOpcode(MI.getOpcode()) || isRMOpcode(MI.getOpcode())) {
      MI.setDesc(TII->get(getRRMOpcodeVariant(MI.getOpcode())));
      if (HasNegOffset) {
        // Change the ALU op (operand 3) from LPAC::ADD (the default) to
        // LPAC::SUB with the already negated offset.
        assert((MI.getOperand(3).getImm() == LPAC::ADD) &&
               "Unexpected ALU op in RRM instruction");
        MI.getOperand(3).setImm(LPAC::SUB);
      }
    } else
      llvm_unreachable("Unexpected opcode in frame index operation");

    MI.getOperand(FIOperandNum).ChangeToRegister(FrameReg, /*isDef=*/false);
    MI.getOperand(FIOperandNum + 1)
        .ChangeToRegister(Reg, /*isDef=*/false, /*isImp=*/false,
                          /*isKill=*/true);
    return false;
  }

  // ALU arithmetic ops take unsigned immediates. If the offset is negative,
  // we replace the instruction with one that inverts the opcode and negates
  // the immediate.
  if ((Offset < 0) && isALUArithLoOpcode(MI.getOpcode())) {
    unsigned NewOpcode = getOppositeALULoOpcode(MI.getOpcode());
    // We know this is an ALU op, so we know the operands are as follows:
    // 0: destination register
    // 1: source register (frame register)
    // 2: immediate
    BuildMI(*MI.getParent(), II, DL, TII->get(NewOpcode),
            MI.getOperand(0).getReg())
        .addReg(FrameReg)
        .addImm(-Offset);
    MI.eraseFromParent();
    return true;
  }

  MI.getOperand(FIOperandNum).ChangeToRegister(FrameReg, /*isDef=*/false);
  MI.getOperand(FIOperandNum + 1).ChangeToImmediate(Offset);
  return false;
}

bool OpenGPURegisterInfo::hasBasePointer(const MachineFunction &MF) const {
  const MachineFrameInfo &MFI = MF.getFrameInfo();
  // When we need stack realignment and there are dynamic allocas, we can't
  // reference off of the stack pointer, so we reserve a base pointer.
  if (hasStackRealignment(MF) && MFI.hasVarSizedObjects())
    return true;

  return false;
}

unsigned OpenGPURegisterInfo::getRARegister() const { return OpenGPU::RCA; }

Register
OpenGPURegisterInfo::getFrameRegister(const MachineFunction & /*MF*/) const {
  return OpenGPU::FP;
}

Register OpenGPURegisterInfo::getBaseRegister() const { return OpenGPU::R14; }

const uint32_t *
OpenGPURegisterInfo::getCallPreservedMask(const MachineFunction & /*MF*/,
                                        CallingConv::ID /*CC*/) const {
  return CSR_RegMask;
}
