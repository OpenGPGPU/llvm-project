# RUN: llvm-mc %s -triple=opengpu -mcpu=opengpu -M no-aliases -show-encoding | FileCheck %s --check-prefixes=INST,ENC
# RUN: llvm-mc %s -filetype=obj -triple=opengpu -mcpu=opengpu | llvm-objdump --triple=opengpu --mcpu=opengpu -M no-aliases -d - | FileCheck %s --check-prefix=OBJ
# RUN: not llvm-mc %s -triple=riscv32 -mattr=-xopengpu 2>&1 | FileCheck %s --check-prefix=NOEXT

# rs1 is a scalar register holding the selector, not an immediate. MC support
# grants no language profile, task binding or general geometry permission.

# INST: opengpu.tess.info a0, a0
# OBJ: opengpu.tess.info a0, a0
# ENC: encoding: [0x0b,0x05,0x05,0x4a]
# NOEXT: :[[@LINE+1]]:1: error: instruction requires the following:
opengpu.tess.info a0, a0

# INST: opengpu.tess.info zero, t6
# OBJ: opengpu.tess.info zero, t6
# ENC: encoding: [0x0b,0x80,0x0f,0x4a]
# NOEXT: :[[@LINE+1]]:1: error: instruction requires the following:
opengpu.tess.info zero, t6

# INST: opengpu.tess.info t6, zero
# OBJ: opengpu.tess.info t6, zero
# ENC: encoding: [0x8b,0x0f,0x00,0x4a]
# NOEXT: :[[@LINE+1]]:1: error: instruction requires the following:
opengpu.tess.info t6, zero
