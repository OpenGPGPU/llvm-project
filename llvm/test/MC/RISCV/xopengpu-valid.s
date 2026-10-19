# XOpengpu custom instructions. Encodings match the Chisel patterns in
# ScalarDecoder.scala / VectorDecoder.scala: custom-0 barrier and tex.sample,
# SYSTEM cease, custom-1 vtex.sample and vquad.dfdx/dfdy, custom-2 vector
# branches (B-type, funct3-selected) and join (funct3=010, other fields zero).
# RUN: llvm-mc %s -triple=opengpu -mcpu=opengpu -M no-aliases -show-encoding \
# RUN:     | FileCheck -check-prefixes=CHECK-ENC,CHECK-INST %s
# RUN: llvm-mc -filetype=obj -triple=opengpu -mcpu=opengpu < %s \
# RUN:     | llvm-objdump --triple=opengpu --mcpu=opengpu -M no-aliases --no-print-imm-hex -d - \
# RUN:     | FileCheck -check-prefix=CHECK-OBJ %s
# RUN: not llvm-mc -triple=riscv32 -mattr=-xopengpu < %s 2>&1 \
# RUN:     | FileCheck -check-prefix=CHECK-NO-EXT %s

# CHECK-INST: opengpu.barrier
# CHECK-OBJ: opengpu.barrier
# CHECK-ENC: encoding: [0x0b,0x00,0x00,0x00]
# CHECK-NO-EXT: :[[@LINE+1]]:1: error: instruction requires the following: 'XOpengpu' (OpenGPU custom instructions)
opengpu.barrier

# CHECK-INST: opengpu.tex.sample a0, ra, sp
# CHECK-OBJ: opengpu.tex.sample a0, ra, sp
# CHECK-ENC: encoding: [0x0b,0x85,0x20,0x02]
# CHECK-NO-EXT: :[[@LINE+1]]:1: error: instruction requires the following: 'XOpengpu' (OpenGPU custom instructions)
opengpu.tex.sample x10, x1, x2

# CHECK-INST: opengpu.cease
# CHECK-OBJ: opengpu.cease
# CHECK-ENC: encoding: [0x73,0x00,0x50,0x30]
# CHECK-NO-EXT: :[[@LINE+1]]:1: error: instruction requires the following: 'XOpengpu' (OpenGPU custom instructions)
opengpu.cease

# CHECK-INST: opengpu.vtex.sample v1, v2, v3
# CHECK-OBJ: opengpu.vtex.sample v1, v2, v3
# CHECK-ENC: encoding: [0xab,0x00,0x31,0x06]
# CHECK-NO-EXT: :[[@LINE+1]]:1: error: instruction requires the following: 'XOpengpu' (OpenGPU custom instructions)
opengpu.vtex.sample v1, v2, v3

# CHECK-INST: opengpu.vquad.dfdx v4, v5
# CHECK-OBJ: opengpu.vquad.dfdx v4, v5
# CHECK-ENC: encoding: [0x2b,0x02,0x50,0x32]
# CHECK-NO-EXT: :[[@LINE+1]]:1: error: instruction requires the following: 'XOpengpu' (OpenGPU custom instructions)
opengpu.vquad.dfdx v4, v5

# CHECK-INST: opengpu.vquad.dfdy v6, v7
# CHECK-OBJ: opengpu.vquad.dfdy v6, v7
# CHECK-ENC: encoding: [0x2b,0x03,0x70,0x36]
# CHECK-NO-EXT: :[[@LINE+1]]:1: error: instruction requires the following: 'XOpengpu' (OpenGPU custom instructions)
opengpu.vquad.dfdy v6, v7

# CHECK-INST: opengpu.vbeq ra, sp, 8
# CHECK-OBJ: opengpu.vbeq ra, sp, 0x20
# CHECK-ENC: encoding: [0x5b,0x84,0x20,0x00]
# CHECK-NO-EXT: :[[@LINE+1]]:1: error: instruction requires the following: 'XOpengpu' (OpenGPU custom instructions)
opengpu.vbeq x1, x2, 8

# CHECK-INST: opengpu.vbne gp, tp, -4
# CHECK-OBJ: opengpu.vbne gp, tp, 0x18
# CHECK-ENC: encoding: [0xdb,0x9e,0x41,0xfe]
# CHECK-NO-EXT: :[[@LINE+1]]:1: error: instruction requires the following: 'XOpengpu' (OpenGPU custom instructions)
opengpu.vbne x3, x4, -4

# CHECK-INST: opengpu.vblt t0, t1, 16
# CHECK-OBJ: opengpu.vblt t0, t1, 0x30
# CHECK-ENC: encoding: [0x5b,0xc8,0x62,0x00]
# CHECK-NO-EXT: :[[@LINE+1]]:1: error: instruction requires the following: 'XOpengpu' (OpenGPU custom instructions)
opengpu.vblt x5, x6, 16

# CHECK-INST: opengpu.vbge t2, s0, 32
# CHECK-OBJ: opengpu.vbge t2, s0, 0x44
# CHECK-ENC: encoding: [0x5b,0xd0,0x83,0x02]
# CHECK-NO-EXT: :[[@LINE+1]]:1: error: instruction requires the following: 'XOpengpu' (OpenGPU custom instructions)
opengpu.vbge x7, x8, 32

# CHECK-INST: opengpu.vbltu s1, a0, 64
# CHECK-OBJ: opengpu.vbltu s1, a0, 0x68
# CHECK-ENC: encoding: [0x5b,0xe0,0xa4,0x04]
# CHECK-NO-EXT: :[[@LINE+1]]:1: error: instruction requires the following: 'XOpengpu' (OpenGPU custom instructions)
opengpu.vbltu x9, x10, 64

# CHECK-INST: opengpu.vbgeu a1, a2, 128
# CHECK-OBJ: opengpu.vbgeu a1, a2, 0xac
# CHECK-ENC: encoding: [0x5b,0xf0,0xc5,0x08]
# CHECK-NO-EXT: :[[@LINE+1]]:1: error: instruction requires the following: 'XOpengpu' (OpenGPU custom instructions)
opengpu.vbgeu x11, x12, 128

# CHECK-INST: opengpu.join
# CHECK-OBJ: opengpu.join
# CHECK-ENC: encoding: [0x5b,0x20,0x00,0x00]
# CHECK-NO-EXT: :[[@LINE+1]]:1: error: instruction requires the following: 'XOpengpu' (OpenGPU custom instructions)
opengpu.join
