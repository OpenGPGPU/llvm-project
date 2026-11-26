//===-- RISCVVectorMaskOperand.td --------------------------------*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

def VMaskAsmOperand : AsmOperandClass {
  let Name = "RVVMaskRegOpOperand";
  let RenderMethod = "addRegOperands";
  let PredicateMethod = "isV0Reg";
  let ParserMethod = "parseMaskReg";
  let IsOptional = 1;
  let DefaultMethod = "defaultMaskRegOp";
  let DiagnosticType = "InvalidVMaskRegister";
  let DiagnosticString = "operand must be v0.t";
}

def VMaskCarryInAsmOperand : AsmOperandClass {
  let Name = "RVVMaskCarryInRegOpOperand";
  let RenderMethod = "addRegOperands";
  let PredicateMethod = "isV0Reg";
  let ParserMethod = "parseMaskReg";
  let DiagnosticType = "InvalidVMaskCarryInRegister";
  let DiagnosticString = "operand must be v0";
}

// An optional v0.t operand encoded in vm.
def VMaskOp : RegisterOperand<VMV0> {
  let ParserMatchClass = VMaskAsmOperand;
  let PrintMethod = "printVMaskReg";
  let EncoderMethod = "getVMaskReg";
  let DecoderMethod = "decodeVMaskReg";
  let OperandNamespace = "RISCVOp";
  let OperandType = "OPERAND_VMASK";
}

// An always present v0 operand encoded with vm=0. Classes that use this must
// set the vm field in RVInstV* to 0.
def VMaskCarryInOp : RegisterOperand<VMV0> {
  let ParserMatchClass = VMaskCarryInAsmOperand;
  let EncoderMethod = "getVMaskReg";
  let DecoderMethod = "decodeVMaskReg";
}
