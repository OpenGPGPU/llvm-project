//===-- RISCVInstrInfoXOpengpuTES.td -----------------------*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

let Predicates = [HasVendorXOpengpu] in {

// opengpu.vtes.patch.read vd, vs1, vs2[, v0.t] - TES task-owned patch
// output inspection. vs1 carries full-width per-lane vertex indices and vs2
// carries output/per-patch/level descriptors; the retained task binding checks
// every selected lane. vm=0 preserves inactive old-vd lanes. This is CUSTOM1
// funct6=33/funct3=0, not CUSTOM0 geometry function 33 output publication.
//
// The read is context-dependent, may fault, and must not be removed, duplicated
// or moved across task bind/finish/cancel or divergent control boundaries.
// MC support does not supply a task binding, grant TES/GS permission or select
// this instruction from generic loads. Future codegen needs explicit owned
// resource/context dependencies in addition to these conservative properties.
let hasSideEffects = 1, mayLoad = 1, mayStore = 0,
    isConvergent = 1, isNotDuplicable = 1 in
def XOPENGPU_VTES_PATCH_READ : RVInstVOpengpu<0b100001, OPC_CUSTOM_1,
        (outs VR:$vd), (ins VR:$vs1, VR:$vs2, VMaskOp:$vm),
        "opengpu.vtes.patch.read", "$vd, $vs1, $vs2$vm">, Sched<[]>;

} // Predicates = [HasVendorXOpengpu]
