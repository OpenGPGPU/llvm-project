//===- OpenGPU.td - Describe the OpenGPU Target Machine --------*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Target-independent interfaces which we are implementing
//===----------------------------------------------------------------------===//

include "llvm/Target/Target.td"

//===----------------------------------------------------------------------===//
// Register File, Calling Conv, Instruction Descriptions
//===----------------------------------------------------------------------===//

include "OpenGPUSchedule.td"
include "OpenGPURegisterInfo.td"
include "OpenGPUCallingConv.td"
include "OpenGPUInstrInfo.td"

defm : RemapAllTargetPseudoPointerOperands<GPR>;

def OpenGPUInstrInfo : InstrInfo;

//===----------------------------------------------------------------------===//
// OpenGPU processors supported.
//===----------------------------------------------------------------------===//

def : ProcessorModel<"generic", OpenGPUSchedModel, []>;
def : ProcessorModel<"v11", OpenGPUSchedModel, []>;

def OpenGPUInstPrinter : AsmWriter {
  string AsmWriterClassName  = "InstPrinter";
  bit isMCAsmWriter = 1;
}

//===----------------------------------------------------------------------===//
// Declare the target which we are implementing
//===----------------------------------------------------------------------===//

def OpenGPU : Target {
  // Pull in Instruction Info:
  let InstructionSet = OpenGPUInstrInfo;
  let AssemblyWriters = [OpenGPUInstPrinter];
}
