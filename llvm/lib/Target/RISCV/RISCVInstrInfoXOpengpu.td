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

include "RISCVInstrFormatsXOpengpu.td"

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

// opengpu.sync join-target - establish an explicit SIMT reconvergence scope.
// The B-immediate names the join instruction; popping its original-mask marker
// resumes at join-target+4. funct3=011 was reserved; rs1 and rs2 must be zero.
// This adds assembler/disassembler support only, not automatic codegen or
// admission to the driver's bounded shader profile.
let hasSideEffects = 1, mayLoad = 0, mayStore = 0 in
def XOPENGPU_SYNC : RVInstB<0b011, OPC_CUSTOM_2, (outs),
                            (ins bare_simm13_lsb0:$imm12),
                            "opengpu.sync", "$imm12">, Sched<[]> {
  let rs1 = 0;
  let rs2 = 0;
  let isBarrier = 1;
}

// Resource-bound typed fetch and captured-operand floating sampling. The
// component lives in funct3; all vector forms retain normal RVV masking.
foreach C = 0-3 in {
  let hasSideEffects = 1, mayLoad = 1, mayStore = 0 in {
    def XOPENGPU_TEXFETCH_#C : RVInstR<0b0000010, C, OPC_CUSTOM_0,
        (outs GPR:$rd), (ins GPR:$rs1, GPR:$rs2),
        "opengpu.tex.fetch." # C, "$rd, $rs1, $rs2">, Sched<[]>;
    def XOPENGPU_TEXSAMPLEF_#C : RVInstR<0b0000100, C, OPC_CUSTOM_0,
        (outs GPR:$rd), (ins GPR:$rs1, GPR:$rs2),
        "opengpu.tex.samplef." # C, "$rd, $rs1, $rs2">, Sched<[]>;
    def XOPENGPU_TEXFETCHI_#C : RVInstR<0b0000101, C, OPC_CUSTOM_0,
        (outs GPR:$rd), (ins GPR:$rs1, GPR:$rs2),
        "opengpu.tex.fetchi." # C, "$rd, $rs1, $rs2">, Sched<[]>;
    def XOPENGPU_VTEX_FETCH_#C : RVInstVOpengpu<0b000010, OPC_CUSTOM_1,
        (outs VR:$vd), (ins VR:$vs1, VR:$vs2, VMaskOp:$vm),
        "opengpu.vtex.fetch." # C, "$vd, $vs1, $vs2$vm">, Sched<[]> {
      let Inst{14-12} = C;
    }
    def XOPENGPU_VTEX_SAMPLEF_#C : RVInstVOpengpu<0b000100, OPC_CUSTOM_1,
        (outs VR:$vd), (ins VR:$vs1, VR:$vs2, VMaskOp:$vm),
        "opengpu.vtex.samplef." # C, "$vd, $vs1, $vs2$vm">, Sched<[]> {
      let Inst{14-12} = C;
    }
    def XOPENGPU_VTEX_FETCHI_#C : RVInstVOpengpu<0b000101, OPC_CUSTOM_1,
        (outs VR:$vd), (ins VR:$vs1, VR:$vs2, VMaskOp:$vm),
        "opengpu.vtex.fetchi." # C, "$vd, $vs1, $vs2$vm">, Sched<[]> {
      let Inst{14-12} = C;
    }
  }
}
// Metadata-only size queries use unit and base-relative LOD from ordinary
// source registers. No captured coordinate tuple is consumed.
foreach C = 0-2 in {
  let hasSideEffects = 1, mayLoad = 1, mayStore = 0 in {
    def XOPENGPU_TEXSIZE_#C : RVInstR<0b0000110, C, OPC_CUSTOM_0,
        (outs GPR:$rd), (ins GPR:$rs1, GPR:$rs2),
        "opengpu.tex.size." # C, "$rd, $rs1, $rs2">, Sched<[]>;
    def XOPENGPU_VTEX_SIZE_#C : RVInstVOpengpu<0b000110, OPC_CUSTOM_1,
        (outs VR:$vd), (ins VR:$vs1, VR:$vs2, VMaskOp:$vm),
        "opengpu.vtex.size." # C, "$vd, $vs1, $vs2$vm">, Sched<[]> {
      let Inst{14-12} = C;
    }
  }
}
// Eight side-effecting operand-pair writes. Encoding rd/vd is zero and is not
// an architectural destination. The texture bank is scoped to a native task.
foreach P = 0-7 in {
  let hasSideEffects = 1, mayLoad = 0, mayStore = 0 in {
    def XOPENGPU_TEXPARAMS_#P : RVInstR<0b0000011, P, OPC_CUSTOM_0,
        (outs), (ins GPR:$rs1, GPR:$rs2),
        "opengpu.tex.params." # P, "$rs1, $rs2">, Sched<[]> {
      let rd = 0;
    }
    def XOPENGPU_VTEX_PARAMS_#P : RVInstVOpengpu<0b000011, OPC_CUSTOM_1,
        (outs), (ins VR:$vs1, VR:$vs2, VMaskOp:$vm),
        "opengpu.vtex.params." # P, "$vs1, $vs2$vm">, Sched<[]> {
      let Inst{14-12} = P;
      let vd = 0;
    }
  }
}

} // Predicates = [HasVendorXOpengpu]
