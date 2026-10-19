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
// Only the scalar custom instructions are defined here.  The custom vector
// forms (vtex.sample, vquad.dfdx/dfdy) and the custom vector-branch/join
// controls are added with the vector support.
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
let hasSideEffects = 1, mayLoad = 0, mayStore = 0 in
def XOPENGPU_CEASE : RVInstR<0b0011000, 0b000, OPC_SYSTEM, (outs), (ins),
                             "opengpu.cease", "">, Sched<[]> {
  let rs2 = 0b00101;
  let rs1 = 0;
  let rd = 0;
}

} // Predicates = [HasVendorXOpengpu]
