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
#include "llvm/Analysis/ConstantFolding.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/Local.h"

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

// At -O0, clang leaves a call for every static helper and a branch for every
// constant `if`. A call is illegal in the device ABI, and the fast allocator
// spills vectors across those branches. Inline the helpers and fold the
// branches before instruction selection. Optimized builds already do both.
namespace {
class RISCVOpenGPUO0Prepare : public ModulePass {
public:
  static char ID;
  RISCVOpenGPUO0Prepare() : ModulePass(ID) {
    initializeRISCVOpenGPUO0PreparePass(*PassRegistry::getPassRegistry());
  }
  bool runOnModule(Module &M) override;
  StringRef getPassName() const override {
    return "OpenGPU -O0 device prepare";
  }
};

static bool inlineDefinedCalls(Function &F) {
  bool Changed = false;
  bool Again = true;
  while (Again) {
    Again = false;
    for (BasicBlock &BB : F) {
      for (Instruction &I : BB) {
        auto *CB = dyn_cast<CallBase>(&I);
        if (!CB || CB->isInlineAsm())
          continue;
        Function *Callee = CB->getCalledFunction();
        if (!Callee || Callee->isDeclaration() || Callee->isIntrinsic())
          continue;
        InlineFunctionInfo IFI;
        if (InlineFunction(*CB, IFI, /*MergeAttributes=*/false,
                           /*CalleeAAR=*/nullptr, /*InsertLifetime=*/false)
                .isSuccess()) {
          Changed = true;
          Again = true;
          break;
        }
      }
      if (Again)
        break;
    }
  }
  return Changed;
}

static bool foldConstants(Function &F) {
  bool Changed = false;
  const DataLayout &DL = F.getDataLayout();
  for (BasicBlock &BB : F) {
    for (Instruction &I : make_early_inc_range(BB)) {
      // Only fold comparisons. Folding arithmetic rewrites loop steps.
      if (!isa<CmpInst>(I))
        continue;
      Constant *C = ConstantFoldInstruction(&I, DL);
      if (!C)
        continue;
      I.replaceAllUsesWith(C);
      I.eraseFromParent();
      Changed = true;
    }
  }
  return Changed;
}

static bool foldTerminators(Function &F) {
  bool Changed = false;
  SmallVector<BasicBlock *, 8> Blocks;
  for (BasicBlock &BB : F)
    Blocks.push_back(&BB);
  for (BasicBlock *BB : Blocks) {
    if (BB->getParent() == &F && ConstantFoldTerminator(BB, true))
      Changed = true;
  }
  if (removeUnreachableBlocks(F))
    Changed = true;
  // FoldSingleEntryPHINodes assumes the caller proved there is one
  // predecessor. Calling it on a loop header replaces the induction phi
  // with the entry value and the loop never counts.
  for (BasicBlock &BB : F)
    if (BB.getSinglePredecessor())
      FoldSingleEntryPHINodes(&BB);
  return Changed;
}
} // end anonymous namespace

char RISCVOpenGPUO0Prepare::ID = 0;
INITIALIZE_PASS(RISCVOpenGPUO0Prepare, "riscv-opengpu-o0-prepare",
                "OpenGPU -O0 device prepare", false, false)

ModulePass *llvm::createRISCVOpenGPUO0PreparePass() {
  return new RISCVOpenGPUO0Prepare();
}

bool RISCVOpenGPUO0Prepare::runOnModule(Module &M) {
  if (Triple(M.getTargetTriple()).getArch() != Triple::opengpu)
    return false;

  bool Changed = false;
  for (unsigned Round = 0; Round != 8; ++Round) {
    bool Progress = false;
    for (Function &F : M) {
      if (F.isDeclaration())
        continue;
      Progress |= inlineDefinedCalls(F);
      Progress |= foldConstants(F);
      Progress |= foldTerminators(F);
    }
    for (Function &F : make_early_inc_range(M)) {
      if (!F.isDeclaration() && F.hasLocalLinkage() && F.use_empty()) {
        F.eraseFromParent();
        Progress = true;
      }
    }
    Changed |= Progress;
    if (!Progress)
      break;
  }
  return Changed;
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
    for (MachineInstr &MI : make_early_inc_range(MBB)) {
      if (MI.isCall()) {
        F.getContext().diagnose(DiagnosticInfoUnsupported{
            F, "OpenGPU shader contains a call, which the device ABI forbids"});
        return false;
      }

      // An unconditional branch to the layout successor is a fallthrough.
      // The sandbox does not admit jal.
      if (MI.getOpcode() == RISCV::PseudoBR && MI.getOperand(0).isMBB() &&
          MI.getOperand(0).getMBB() == MBB.getNextNode()) {
        MI.eraseFromParent();
        Changed = true;
        continue;
      }

      // The device does not implement whole-register moves. An m1 copy under
      // the current VL is vmv.v.v.
      if (MI.getOpcode() == RISCV::VMV1R_V) {
        MI.setDesc(TII.get(RISCV::VMV_V_V));
        Changed = true;
      }

      // vsetvli x0, x0 keeps VL and only rewrites vtype. After the policy bits
      // are cleared it duplicates the previous e32,m1 tu,mu configuration, and
      // the sandbox rejects that encoding.
      if ((MI.getOpcode() == RISCV::VSETVLI ||
           MI.getOpcode() == RISCV::PseudoVSETVLIX0X0) &&
          MI.getOperand(0).isReg() && MI.getOperand(1).isReg() &&
          MI.getOperand(0).getReg() == RISCV::X0 &&
          MI.getOperand(1).getReg() == RISCV::X0) {
        MI.eraseFromParent();
        Changed = true;
        continue;
      }

      // Force tail- and mask-undisturbed vtype (vta = vma = 0).  The device
      // sandbox only admits the e32,m1 tu,mu configuration, and tu/mu is a
      // strictly stronger guarantee than the ta/ma that LLVM emits by default.
      switch (MI.getOpcode()) {
      case RISCV::PseudoVSETVLI:
      case RISCV::PseudoVSETVLIX0:
      case RISCV::PseudoVSETVLIX0X0:
      case RISCV::PseudoVSETIVLI:
      case RISCV::VSETVLI:
      case RISCV::VSETIVLI: {
        MachineOperand &VType = MI.getOperand(2);
        int64_t Imm = VType.getImm();
        if (Imm & 0xC0) {
          VType.setImm(Imm & ~0xC0);
          Changed = true;
        }
        break;
      }
      default:
        break;
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
