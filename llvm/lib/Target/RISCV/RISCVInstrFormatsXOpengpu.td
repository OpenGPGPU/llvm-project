//===-- RISCVInstrFormatsXOpengpu.td --------------------------------*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

// Custom vector instructions share the RVV register layout (funct6, vm, vs2,
// vs1, funct3, vd, opcode) but live in a custom opcode space.  The base class
// mirrors RVInstVBase with a configurable opcode.  The driver profile only
// emits the unmasked (vm=1 for vtex.sample, vs1=0 for the unary vquad forms).
class RVInstVOpengpu<bits<6> funct6, RISCVOpcode opcode, dag outs, dag ins,
                     string opcodestr, string argstr>
    : RVInst<outs, ins, opcodestr, argstr, [], InstFormatR> {
  bits<5> vd;
  bit vm;
  bits<5> vs2;
  bits<5> vs1;

  let Inst{31-26} = funct6;
  let Inst{25} = vm;
  let Inst{24-20} = vs2;
  let Inst{19-15} = vs1;
  let Inst{14-12} = 0b000;
  let Inst{11-7} = vd;
  let Inst{6-0} = opcode.Value;

  let Uses = [VL, VTYPE];
}

