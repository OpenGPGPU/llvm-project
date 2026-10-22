# RUN: not llvm-mc %s -triple=opengpu -mcpu=opengpu 2>&1 | FileCheck %s

# CHECK: :[[@LINE+1]]:14: error: immediate must be a multiple of 2 bytes in the range [-4096, 4094]
opengpu.sync 4096

# CHECK: :[[@LINE+1]]:14: error: immediate must be a multiple of 2 bytes in the range [-4096, 4094]
opengpu.sync -4098

# CHECK: :[[@LINE+1]]:14: error: immediate must be a multiple of 2 bytes in the range [-4096, 4094]
opengpu.sync 1
