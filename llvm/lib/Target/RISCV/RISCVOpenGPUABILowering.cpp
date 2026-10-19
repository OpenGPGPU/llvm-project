//===-- RISCVOpenGPUABILowering.cpp - OpenGPU device ABI lowering --------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This pass realizes the OpenGPU device calling convention for the `opengpu`
// triple.  The device ABI has no stack, no calls, and no conventional return:
// a warp permanently terminates with `opengpu.cease`.  The kernel body runs
// with the runtime-preloaded registers (x1 = kernarg base, x30/x31 reserved)
// and the shader sandbox in driver/opengpu_shader_validator.h rejects anything
// else.
//
// The pass rewrites every `PseudoRET` into `opengpu.cease` and diagnoses
// constructs the sandbox cannot admit (stack objects and calls).  x8 is left
// untouched here because the trusted localLinearBase is supplied at launch.
//
//===----------------------------------------------------------------------===//

#include "RISCV.h"
#include "RISCVSubtarget.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/IR/DiagnosticInfo.h"

using namespace llvm;

#define DEBUG_TYPE "riscv-opengpu-abi"
#define RISCV_OPENGPU_ABI_NAME "OpenGPU device ABI lowering"

namespace {
class RISCVOpenGPUABILowering : public MachineFunctionPass {
public:
  static char ID;

  RISCVOpenGPUABILowering() : MachineFunctionPass(ID) {
    initializeRISCVOpenGPUABILoweringPass(*PassRegistry::getPassRegistry());
  }

  bool runOnMachineFunction(MachineFunction &MF) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
    MachineFunctionPass::getAnalysisUsage(AU);
  }

  StringRef getPassName() const override { return RISCV_OPENGPU_ABI_NAME; }
};
} // end anonymous namespace

char RISCVOpenGPUABILowering::ID = 0;
INITIALIZE_PASS(RISCVOpenGPUABILowering, DEBUG_TYPE, RISCV_OPENGPU_ABI_NAME,
                false, false)

FunctionPass *llvm::createRISCVOpenGPUABILoweringPass() {
  return new RISCVOpenGPUABILowering();
}

bool RISCVOpenGPUABILowering::runOnMachineFunction(MachineFunction &MF) {
  const RISCVSubtarget &ST = MF.getSubtarget<RISCVSubtarget>();
  if (!ST.isOpenGPU() || skipFunction(MF.getFunction()))
    return false;

  const TargetInstrInfo &TII = *ST.getInstrInfo();
  const Function &F = MF.getFunction();

  // The device ABI has no stack and no calls.  Diagnose rather than emit code
  // that the shader sandbox would reject at load time.
  if (MF.getFrameInfo().hasStackObjects()) {
    F.getContext().diagnose(DiagnosticInfoUnsupported{
        F, "OpenGPU shader allocates stack, which the device ABI forbids"});
    return false;
  }

  bool Changed = false;
  for (MachineBasicBlock &MBB : MF) {
    for (MachineInstr &MI : MBB) {
      if (MI.isCall()) {
        F.getContext().diagnose(DiagnosticInfoUnsupported{
            F, "OpenGPU shader contains a call, which the device ABI forbids"});
        return false;
      }
    }
    for (MachineBasicBlock::iterator I = MBB.getFirstTerminator();
         I != MBB.end(); ++I) {
      if (I->getOpcode() != RISCV::PseudoRET)
        continue;
      DebugLoc DL = I->getDebugLoc();
      BuildMI(MBB, I, DL, TII.get(RISCV::XOPENGPU_CEASE));
      I->eraseFromParent();
      Changed = true;
      break;
    }
  }

  return Changed;
}
