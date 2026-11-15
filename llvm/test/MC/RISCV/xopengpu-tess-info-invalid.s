# RUN: not llvm-mc %s -triple=opengpu -mcpu=opengpu 2>&1 | FileCheck %s

# CHECK: :[[@LINE+1]]:{{[0-9]+}}: error:
opengpu.tess.info v10, a0
# CHECK: :[[@LINE+1]]:{{[0-9]+}}: error:
opengpu.tess.info a0, v10
# CHECK: :[[@LINE+1]]:{{[0-9]+}}: error:
opengpu.tess.info a0, 6
# CHECK: :[[@LINE+1]]:{{[0-9]+}}: error:
opengpu.tess.info a0
# CHECK: :[[@LINE+1]]:{{[0-9]+}}: error:
opengpu.tess.info a0, a0, zero
