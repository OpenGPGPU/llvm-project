//===-- RISCVInstrInfoXOpengpu.td -------------------------*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file describes the custom instructions of the OpenGPU SIMT GPU.
//
// The OpenGPU device ISA is RV32IMAFV (plus Zicsr/Zifencei and atomics) with a
// small set of GPU-specific custom instructions.  The authoritative encodings
// live in the hardware decoders under
// src/main/scala/opengpu/core/frontend/decode in the OpenGPU repository.
//
// Scalar customs, the custom-1 vector forms, and the custom-2 vector-branch
// family plus join are defined here.  The branch/join controls are decoded by
// the device but are not selected by codegen: their execution units are not
// part of the shader ABI.
//
//===----------------------------------------------------------------------===//

let Predicates = [HasVendorXOpengpu] in {

// opengpu.barrier - workgroup barrier executed by every active lane.
// Encoded in the RISC-V custom-0 opcode space with all other fields zero.
let hasSideEffects = 1, mayLoad = 0, mayStore = 0 in
def XOPENGPU_BARRIER : RVInstR<0b0000000, 0b000, OPC_CUSTOM_0, (outs), (ins),
                               "opengpu.barrier", "">, Sched<[]> {
  let rs2 = 0;
  let rs1 = 0;
  let rd = 0;
}

// opengpu.tex.sample rd, rs1, rs2 - warp-uniform texture sample of the bound
// texture at (u=rs1, v=rs2), writing the packed RGBA8888 texel word to rd.
// Encoded in the custom-0 opcode space, funct7=0000001, funct3=000.
let hasSideEffects = 1, mayLoad = 1, mayStore = 0 in
def XOPENGPU_TEXSAMPLE : RVInstR<0b0000001, 0b000, OPC_CUSTOM_0,
                                 (outs GPR:$rd), (ins GPR:$rs1, GPR:$rs2),
                                 "opengpu.tex.sample", "$rd, $rs1, $rs2">,
                         Sched<[]>;

// opengpu.cease - permanently terminate the executing warp.
// Encoded in the SYSTEM opcode space, funct7=0011000, rs2=00101.
// It ends the shader path, so it is modelled as a return-like terminator.
let hasSideEffects = 1, mayLoad = 0, mayStore = 0 in
def XOPENGPU_CEASE : RVInstR<0b0011000, 0b000, OPC_SYSTEM, (outs), (ins),
                             "opengpu.cease", "">, Sched<[]> {
  let rs2 = 0b00101;
  let rs1 = 0;
  let rd = 0;
  let isBarrier = 1;
  let isReturn = 1;
  let isTerminator = 1;
}

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

// opengpu.vtex.sample vd, vs1, vs2 - per-lane texture sample of the bound
// texture at (u=vs1, v=vs2), writing packed RGBA8888 texels to vd.
let hasSideEffects = 1, mayLoad = 1, mayStore = 0 in
def XOPENGPU_VTEX_SAMPLE : RVInstVOpengpu<0b000001, OPC_CUSTOM_1,
                                         (outs VR:$vd), (ins VR:$vs1, VR:$vs2),
                                         "opengpu.vtex.sample",
                                         "$vd, $vs1, $vs2">, Sched<[]> {
  let vm = 1;
}

// opengpu.vquad.dfdx vd, vs2 - fragment-quad horizontal derivative.
let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
def XOPENGPU_VQUAD_DFDX : RVInstVOpengpu<0b001100, OPC_CUSTOM_1,
                                        (outs VR:$vd), (ins VR:$vs2),
                                        "opengpu.vquad.dfdx", "$vd, $vs2">,
                         Sched<[]> {
  let vm = 1;
  let vs1 = 0;
}

// opengpu.vquad.dfdy vd, vs2 - fragment-quad vertical derivative.
let hasSideEffects = 0, mayLoad = 0, mayStore = 0 in
def XOPENGPU_VQUAD_DFDY : RVInstVOpengpu<0b001101, OPC_CUSTOM_1,
                                        (outs VR:$vd), (ins VR:$vs2),
                                        "opengpu.vquad.dfdy", "$vd, $vs2">,
                         Sched<[]> {
  let vm = 1;
  let vs1 = 0;
}

// Custom-2 (opcode 0x5b) vector branches.  The Chisel pattern is
// ?????????????????funct3?????1011011 with a B-type immediate, rs1 and rs2.
// funct3 selects the condition the same way as a scalar branch.
class RVInstVBranch<bits<3> funct3, string opcodestr>
    : RVInstB<funct3, OPC_CUSTOM_2, (outs),
              (ins GPR:$rs1, GPR:$rs2, bare_simm13_lsb0:$imm12),
              opcodestr, "$rs1, $rs2, $imm12">, Sched<[]> {
  let isBranch = 1;
  let isTerminator = 1;
  let hasSideEffects = 1;
  let mayLoad = 0;
  let mayStore = 0;
}

def XOPENGPU_VBEQ  : RVInstVBranch<0b000, "opengpu.vbeq">;
def XOPENGPU_VBNE  : RVInstVBranch<0b001, "opengpu.vbne">;
def XOPENGPU_VBLT  : RVInstVBranch<0b100, "opengpu.vblt">;
def XOPENGPU_VBGE  : RVInstVBranch<0b101, "opengpu.vbge">;
def XOPENGPU_VBLTU : RVInstVBranch<0b110, "opengpu.vbltu">;
def XOPENGPU_VBGEU : RVInstVBranch<0b111, "opengpu.vbgeu">;

// opengpu.join - SIMT reconvergence.  Chisel matches funct3=010 and opcode
// 0x5b; every other field is ignored.  The canonical encoding clears them.
let hasSideEffects = 1, mayLoad = 0, mayStore = 0 in
def XOPENGPU_JOIN : RVInstR<0b0000000, 0b010, OPC_CUSTOM_2, (outs), (ins),
                            "opengpu.join", "">, Sched<[]> {
  let rs2 = 0;
  let rs1 = 0;
  let rd = 0;
  let isBarrier = 1;
}

} // Predicates = [HasVendorXOpengpu]
