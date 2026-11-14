# RUN: llvm-mc %s -triple=opengpu -mcpu=opengpu -M no-aliases -show-encoding | FileCheck %s --check-prefixes=INST,ENC
# RUN: llvm-mc %s -filetype=obj -triple=opengpu -mcpu=opengpu | llvm-objdump --triple=opengpu --mcpu=opengpu -M no-aliases -d - | FileCheck %s --check-prefix=OBJ
# RUN: not llvm-mc %s -triple=riscv32 -mattr=-xopengpu 2>&1 | FileCheck %s --check-prefix=NOEXT

# MC support alone does not grant a program/task profile or validate its
# layout, VL, old-vd definedness, selected indices, ownership or retirement.

# INST: opengpu.vtes.patch.read v10, v11, v12
# OBJ: opengpu.vtes.patch.read v10, v11, v12
# ENC: encoding: [0x2b,0x85,0xc5,0x86]
# NOEXT: :[[@LINE+1]]:1: error: instruction requires the following:
opengpu.vtes.patch.read v10, v11, v12

# INST: opengpu.vtes.patch.read v10, v11, v12, v0.t
# OBJ: opengpu.vtes.patch.read v10, v11, v12, v0.t
# ENC: encoding: [0x2b,0x85,0xc5,0x84]
# NOEXT: :[[@LINE+1]]:1: error: instruction requires the following:
opengpu.vtes.patch.read v10, v11, v12, v0.t

# INST: opengpu.vtes.patch.read v31, v0, v31
# OBJ: opengpu.vtes.patch.read v31, v0, v31
# ENC: encoding: [0xab,0x0f,0xf0,0x87]
# NOEXT: :[[@LINE+1]]:1: error: instruction requires the following:
opengpu.vtes.patch.read v31, v0, v31

# INST: opengpu.vtes.patch.read v0, v31, v0
# OBJ: opengpu.vtes.patch.read v0, v31, v0
# ENC: encoding: [0x2b,0x80,0x0f,0x86]
# NOEXT: :[[@LINE+1]]:1: error: instruction requires the following:
opengpu.vtes.patch.read v0, v31, v0

# INST: opengpu.vtes.patch.read v31, v31, v31, v0.t
# OBJ: opengpu.vtes.patch.read v31, v31, v31, v0.t
# ENC: encoding: [0xab,0x8f,0xff,0x85]
# NOEXT: :[[@LINE+1]]:1: error: instruction requires the following:
opengpu.vtes.patch.read v31, v31, v31, v0.t
