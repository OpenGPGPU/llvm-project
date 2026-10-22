# RUN: llvm-mc %s -filetype=obj -triple=opengpu -mcpu=opengpu | llvm-objdump --triple=opengpu --mcpu=opengpu -M no-aliases -d - | FileCheck %s

.option norvc
.Lstart:
# CHECK: 0: 5b 34 00 00 {{[[:space:]]*}}opengpu.sync 0x8
opengpu.sync .Ljoin
addi x0, x0, 0
.Ljoin:
opengpu.join
# CHECK: c: db 3a 00 fe {{[[:space:]]*}}opengpu.sync 0x0
opengpu.sync .Lstart
