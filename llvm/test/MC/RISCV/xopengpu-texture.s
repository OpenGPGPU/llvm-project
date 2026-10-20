# RUN: llvm-mc %s -triple=opengpu -mcpu=opengpu -M no-aliases -show-encoding | FileCheck %s --check-prefixes=INST,ENC
# RUN: llvm-mc %s -filetype=obj -triple=opengpu -mcpu=opengpu | llvm-objdump --triple=opengpu --mcpu=opengpu -M no-aliases --no-print-imm-hex -d - | FileCheck %s --check-prefix=OBJ
# RUN: not llvm-mc %s -triple=riscv32 -mattr=-xopengpu 2>&1 | FileCheck %s --check-prefix=NOEXT

# INST: opengpu.tex.fetch.0 a0, a1, a2
# OBJ: opengpu.tex.fetch.0 a0, a1, a2
# ENC: encoding: [0x0b,0x85,0xc5,0x04]
# NOEXT: :[[@LINE+1]]:1: error: instruction requires the following:
opengpu.tex.fetch.0 x10, x11, x12

# INST: opengpu.vtex.fetch.0 v10, v11, v12
# OBJ: opengpu.vtex.fetch.0 v10, v11, v12
# ENC: encoding: [0x2b,0x85,0xc5,0x0a]
# NOEXT: :[[@LINE+1]]:1: error: instruction requires the following:
opengpu.vtex.fetch.0 v10, v11, v12

# INST: opengpu.vtex.fetch.0 v10, v11, v12, v0.t
# OBJ: opengpu.vtex.fetch.0 v10, v11, v12, v0.t
# ENC: encoding: [0x2b,0x85,0xc5,0x08]
# NOEXT: :[[@LINE+1]]:1: error: instruction requires the following:
opengpu.vtex.fetch.0 v10, v11, v12, v0.t

# INST: opengpu.tex.fetch.1 a0, a1, a2
# OBJ: opengpu.tex.fetch.1 a0, a1, a2
# ENC: encoding: [0x0b,0x95,0xc5,0x04]
# NOEXT: :[[@LINE+1]]:1: error: instruction requires the following:
opengpu.tex.fetch.1 x10, x11, x12

# INST: opengpu.vtex.fetch.1 v10, v11, v12
# OBJ: opengpu.vtex.fetch.1 v10, v11, v12
# ENC: encoding: [0x2b,0x95,0xc5,0x0a]
# NOEXT: :[[@LINE+1]]:1: error: instruction requires the following:
opengpu.vtex.fetch.1 v10, v11, v12

# INST: opengpu.vtex.fetch.1 v10, v11, v12, v0.t
# OBJ: opengpu.vtex.fetch.1 v10, v11, v12, v0.t
# ENC: encoding: [0x2b,0x95,0xc5,0x08]
# NOEXT: :[[@LINE+1]]:1: error: instruction requires the following:
opengpu.vtex.fetch.1 v10, v11, v12, v0.t

# INST: opengpu.tex.fetch.2 a0, a1, a2
# OBJ: opengpu.tex.fetch.2 a0, a1, a2
# ENC: encoding: [0x0b,0xa5,0xc5,0x04]
# NOEXT: :[[@LINE+1]]:1: error: instruction requires the following:
opengpu.tex.fetch.2 x10, x11, x12

# INST: opengpu.vtex.fetch.2 v10, v11, v12
# OBJ: opengpu.vtex.fetch.2 v10, v11, v12
# ENC: encoding: [0x2b,0xa5,0xc5,0x0a]
# NOEXT: :[[@LINE+1]]:1: error: instruction requires the following:
opengpu.vtex.fetch.2 v10, v11, v12

# INST: opengpu.vtex.fetch.2 v10, v11, v12, v0.t
# OBJ: opengpu.vtex.fetch.2 v10, v11, v12, v0.t
# ENC: encoding: [0x2b,0xa5,0xc5,0x08]
# NOEXT: :[[@LINE+1]]:1: error: instruction requires the following:
opengpu.vtex.fetch.2 v10, v11, v12, v0.t

# INST: opengpu.tex.fetch.3 a0, a1, a2
# OBJ: opengpu.tex.fetch.3 a0, a1, a2
# ENC: encoding: [0x0b,0xb5,0xc5,0x04]
# NOEXT: :[[@LINE+1]]:1: error: instruction requires the following:
opengpu.tex.fetch.3 x10, x11, x12

# INST: opengpu.vtex.fetch.3 v10, v11, v12
# OBJ: opengpu.vtex.fetch.3 v10, v11, v12
# ENC: encoding: [0x2b,0xb5,0xc5,0x0a]
# NOEXT: :[[@LINE+1]]:1: error: instruction requires the following:
opengpu.vtex.fetch.3 v10, v11, v12

# INST: opengpu.vtex.fetch.3 v10, v11, v12, v0.t
# OBJ: opengpu.vtex.fetch.3 v10, v11, v12, v0.t
# ENC: encoding: [0x2b,0xb5,0xc5,0x08]
# NOEXT: :[[@LINE+1]]:1: error: instruction requires the following:
opengpu.vtex.fetch.3 v10, v11, v12, v0.t

# INST: opengpu.tex.samplef.0 a0, a1, a2
# OBJ: opengpu.tex.samplef.0 a0, a1, a2
# ENC: encoding: [0x0b,0x85,0xc5,0x08]
# NOEXT: :[[@LINE+1]]:1: error: instruction requires the following:
opengpu.tex.samplef.0 x10, x11, x12

# INST: opengpu.vtex.samplef.0 v10, v11, v12
# OBJ: opengpu.vtex.samplef.0 v10, v11, v12
# ENC: encoding: [0x2b,0x85,0xc5,0x12]
# NOEXT: :[[@LINE+1]]:1: error: instruction requires the following:
opengpu.vtex.samplef.0 v10, v11, v12

# INST: opengpu.vtex.samplef.0 v10, v11, v12, v0.t
# OBJ: opengpu.vtex.samplef.0 v10, v11, v12, v0.t
# ENC: encoding: [0x2b,0x85,0xc5,0x10]
# NOEXT: :[[@LINE+1]]:1: error: instruction requires the following:
opengpu.vtex.samplef.0 v10, v11, v12, v0.t

# INST: opengpu.tex.samplef.1 a0, a1, a2
# OBJ: opengpu.tex.samplef.1 a0, a1, a2
# ENC: encoding: [0x0b,0x95,0xc5,0x08]
# NOEXT: :[[@LINE+1]]:1: error: instruction requires the following:
opengpu.tex.samplef.1 x10, x11, x12

# INST: opengpu.vtex.samplef.1 v10, v11, v12
# OBJ: opengpu.vtex.samplef.1 v10, v11, v12
# ENC: encoding: [0x2b,0x95,0xc5,0x12]
# NOEXT: :[[@LINE+1]]:1: error: instruction requires the following:
opengpu.vtex.samplef.1 v10, v11, v12

# INST: opengpu.vtex.samplef.1 v10, v11, v12, v0.t
# OBJ: opengpu.vtex.samplef.1 v10, v11, v12, v0.t
# ENC: encoding: [0x2b,0x95,0xc5,0x10]
# NOEXT: :[[@LINE+1]]:1: error: instruction requires the following:
opengpu.vtex.samplef.1 v10, v11, v12, v0.t

# INST: opengpu.tex.samplef.2 a0, a1, a2
# OBJ: opengpu.tex.samplef.2 a0, a1, a2
# ENC: encoding: [0x0b,0xa5,0xc5,0x08]
# NOEXT: :[[@LINE+1]]:1: error: instruction requires the following:
opengpu.tex.samplef.2 x10, x11, x12

# INST: opengpu.vtex.samplef.2 v10, v11, v12
# OBJ: opengpu.vtex.samplef.2 v10, v11, v12
# ENC: encoding: [0x2b,0xa5,0xc5,0x12]
# NOEXT: :[[@LINE+1]]:1: error: instruction requires the following:
opengpu.vtex.samplef.2 v10, v11, v12

# INST: opengpu.vtex.samplef.2 v10, v11, v12, v0.t
# OBJ: opengpu.vtex.samplef.2 v10, v11, v12, v0.t
# ENC: encoding: [0x2b,0xa5,0xc5,0x10]
# NOEXT: :[[@LINE+1]]:1: error: instruction requires the following:
opengpu.vtex.samplef.2 v10, v11, v12, v0.t

# INST: opengpu.tex.samplef.3 a0, a1, a2
# OBJ: opengpu.tex.samplef.3 a0, a1, a2
# ENC: encoding: [0x0b,0xb5,0xc5,0x08]
# NOEXT: :[[@LINE+1]]:1: error: instruction requires the following:
opengpu.tex.samplef.3 x10, x11, x12

# INST: opengpu.vtex.samplef.3 v10, v11, v12
# OBJ: opengpu.vtex.samplef.3 v10, v11, v12
# ENC: encoding: [0x2b,0xb5,0xc5,0x12]
# NOEXT: :[[@LINE+1]]:1: error: instruction requires the following:
opengpu.vtex.samplef.3 v10, v11, v12

# INST: opengpu.vtex.samplef.3 v10, v11, v12, v0.t
# OBJ: opengpu.vtex.samplef.3 v10, v11, v12, v0.t
# ENC: encoding: [0x2b,0xb5,0xc5,0x10]
# NOEXT: :[[@LINE+1]]:1: error: instruction requires the following:
opengpu.vtex.samplef.3 v10, v11, v12, v0.t

# INST: opengpu.tex.params.0 a1, a2
# OBJ: opengpu.tex.params.0 a1, a2
# ENC: encoding: [0x0b,0x80,0xc5,0x06]
# NOEXT: :[[@LINE+1]]:1: error: instruction requires the following:
opengpu.tex.params.0 x11, x12

# INST: opengpu.vtex.params.0 v11, v12
# OBJ: opengpu.vtex.params.0 v11, v12
# ENC: encoding: [0x2b,0x80,0xc5,0x0e]
# NOEXT: :[[@LINE+1]]:1: error: instruction requires the following:
opengpu.vtex.params.0 v11, v12

# INST: opengpu.vtex.params.0 v11, v12, v0.t
# OBJ: opengpu.vtex.params.0 v11, v12, v0.t
# ENC: encoding: [0x2b,0x80,0xc5,0x0c]
# NOEXT: :[[@LINE+1]]:1: error: instruction requires the following:
opengpu.vtex.params.0 v11, v12, v0.t

# INST: opengpu.tex.params.1 a1, a2
# OBJ: opengpu.tex.params.1 a1, a2
# ENC: encoding: [0x0b,0x90,0xc5,0x06]
# NOEXT: :[[@LINE+1]]:1: error: instruction requires the following:
opengpu.tex.params.1 x11, x12

# INST: opengpu.vtex.params.1 v11, v12
# OBJ: opengpu.vtex.params.1 v11, v12
# ENC: encoding: [0x2b,0x90,0xc5,0x0e]
# NOEXT: :[[@LINE+1]]:1: error: instruction requires the following:
opengpu.vtex.params.1 v11, v12

# INST: opengpu.vtex.params.1 v11, v12, v0.t
# OBJ: opengpu.vtex.params.1 v11, v12, v0.t
# ENC: encoding: [0x2b,0x90,0xc5,0x0c]
# NOEXT: :[[@LINE+1]]:1: error: instruction requires the following:
opengpu.vtex.params.1 v11, v12, v0.t

# INST: opengpu.tex.params.2 a1, a2
# OBJ: opengpu.tex.params.2 a1, a2
# ENC: encoding: [0x0b,0xa0,0xc5,0x06]
# NOEXT: :[[@LINE+1]]:1: error: instruction requires the following:
opengpu.tex.params.2 x11, x12

# INST: opengpu.vtex.params.2 v11, v12
# OBJ: opengpu.vtex.params.2 v11, v12
# ENC: encoding: [0x2b,0xa0,0xc5,0x0e]
# NOEXT: :[[@LINE+1]]:1: error: instruction requires the following:
opengpu.vtex.params.2 v11, v12

# INST: opengpu.vtex.params.2 v11, v12, v0.t
# OBJ: opengpu.vtex.params.2 v11, v12, v0.t
# ENC: encoding: [0x2b,0xa0,0xc5,0x0c]
# NOEXT: :[[@LINE+1]]:1: error: instruction requires the following:
opengpu.vtex.params.2 v11, v12, v0.t

# INST: opengpu.tex.params.3 a1, a2
# OBJ: opengpu.tex.params.3 a1, a2
# ENC: encoding: [0x0b,0xb0,0xc5,0x06]
# NOEXT: :[[@LINE+1]]:1: error: instruction requires the following:
opengpu.tex.params.3 x11, x12

# INST: opengpu.vtex.params.3 v11, v12
# OBJ: opengpu.vtex.params.3 v11, v12
# ENC: encoding: [0x2b,0xb0,0xc5,0x0e]
# NOEXT: :[[@LINE+1]]:1: error: instruction requires the following:
opengpu.vtex.params.3 v11, v12

# INST: opengpu.vtex.params.3 v11, v12, v0.t
# OBJ: opengpu.vtex.params.3 v11, v12, v0.t
# ENC: encoding: [0x2b,0xb0,0xc5,0x0c]
# NOEXT: :[[@LINE+1]]:1: error: instruction requires the following:
opengpu.vtex.params.3 v11, v12, v0.t

# INST: opengpu.tex.params.4 a1, a2
# OBJ: opengpu.tex.params.4 a1, a2
# ENC: encoding: [0x0b,0xc0,0xc5,0x06]
# NOEXT: :[[@LINE+1]]:1: error: instruction requires the following:
opengpu.tex.params.4 x11, x12

# INST: opengpu.vtex.params.4 v11, v12
# OBJ: opengpu.vtex.params.4 v11, v12
# ENC: encoding: [0x2b,0xc0,0xc5,0x0e]
# NOEXT: :[[@LINE+1]]:1: error: instruction requires the following:
opengpu.vtex.params.4 v11, v12

# INST: opengpu.vtex.params.4 v11, v12, v0.t
# OBJ: opengpu.vtex.params.4 v11, v12, v0.t
# ENC: encoding: [0x2b,0xc0,0xc5,0x0c]
# NOEXT: :[[@LINE+1]]:1: error: instruction requires the following:
opengpu.vtex.params.4 v11, v12, v0.t

# INST: opengpu.tex.params.5 a1, a2
# OBJ: opengpu.tex.params.5 a1, a2
# ENC: encoding: [0x0b,0xd0,0xc5,0x06]
# NOEXT: :[[@LINE+1]]:1: error: instruction requires the following:
opengpu.tex.params.5 x11, x12

# INST: opengpu.vtex.params.5 v11, v12
# OBJ: opengpu.vtex.params.5 v11, v12
# ENC: encoding: [0x2b,0xd0,0xc5,0x0e]
# NOEXT: :[[@LINE+1]]:1: error: instruction requires the following:
opengpu.vtex.params.5 v11, v12

# INST: opengpu.vtex.params.5 v11, v12, v0.t
# OBJ: opengpu.vtex.params.5 v11, v12, v0.t
# ENC: encoding: [0x2b,0xd0,0xc5,0x0c]
# NOEXT: :[[@LINE+1]]:1: error: instruction requires the following:
opengpu.vtex.params.5 v11, v12, v0.t

# INST: opengpu.tex.params.6 a1, a2
# OBJ: opengpu.tex.params.6 a1, a2
# ENC: encoding: [0x0b,0xe0,0xc5,0x06]
# NOEXT: :[[@LINE+1]]:1: error: instruction requires the following:
opengpu.tex.params.6 x11, x12

# INST: opengpu.vtex.params.6 v11, v12
# OBJ: opengpu.vtex.params.6 v11, v12
# ENC: encoding: [0x2b,0xe0,0xc5,0x0e]
# NOEXT: :[[@LINE+1]]:1: error: instruction requires the following:
opengpu.vtex.params.6 v11, v12

# INST: opengpu.vtex.params.6 v11, v12, v0.t
# OBJ: opengpu.vtex.params.6 v11, v12, v0.t
# ENC: encoding: [0x2b,0xe0,0xc5,0x0c]
# NOEXT: :[[@LINE+1]]:1: error: instruction requires the following:
opengpu.vtex.params.6 v11, v12, v0.t

# INST: opengpu.tex.params.7 a1, a2
# OBJ: opengpu.tex.params.7 a1, a2
# ENC: encoding: [0x0b,0xf0,0xc5,0x06]
# NOEXT: :[[@LINE+1]]:1: error: instruction requires the following:
opengpu.tex.params.7 x11, x12

# INST: opengpu.vtex.params.7 v11, v12
# OBJ: opengpu.vtex.params.7 v11, v12
# ENC: encoding: [0x2b,0xf0,0xc5,0x0e]
# NOEXT: :[[@LINE+1]]:1: error: instruction requires the following:
opengpu.vtex.params.7 v11, v12

# INST: opengpu.vtex.params.7 v11, v12, v0.t
# OBJ: opengpu.vtex.params.7 v11, v12, v0.t
# ENC: encoding: [0x2b,0xf0,0xc5,0x0c]
# NOEXT: :[[@LINE+1]]:1: error: instruction requires the following:
opengpu.vtex.params.7 v11, v12, v0.t
