# RUN: llvm-mc %s -triple=opengpu -mcpu=opengpu -M no-aliases -show-encoding | FileCheck %s --check-prefixes=INST,ENC
# RUN: llvm-mc %s -filetype=obj -triple=opengpu -mcpu=opengpu | llvm-objdump --triple=opengpu --mcpu=opengpu -M no-aliases --no-print-imm-hex -d - | FileCheck %s --check-prefix=OBJ
# RUN: not llvm-mc %s -triple=riscv32 -mattr=-xopengpu 2>&1 | FileCheck %s --check-prefix=NOEXT

# INST: opengpu.sync 32
# OBJ: opengpu.sync 0x20
# ENC: encoding: [0x5b,0x30,0x00,0x02]
# NOEXT: :[[@LINE+1]]:1: error: instruction requires the following:
opengpu.sync 32

# INST: opengpu.sync -4096
# OBJ: opengpu.sync 0xfffff004
# ENC: encoding: [0x5b,0x30,0x00,0x80]
# NOEXT: :[[@LINE+1]]:1: error: instruction requires the following:
opengpu.sync -4096

# INST: opengpu.sync 4094
# OBJ: opengpu.sync 0x1006
# ENC: encoding: [0xdb,0x3f,0x00,0x7e]
# NOEXT: :[[@LINE+1]]:1: error: instruction requires the following:
opengpu.sync 4094
