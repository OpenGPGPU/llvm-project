# RUN: not llvm-mc %s -triple=opengpu -mcpu=opengpu 2>&1 | FileCheck %s

# CHECK: :[[@LINE+1]]:{{[0-9]+}}: error:
opengpu.vtes.patch.read x10, v11, v12

# CHECK: :[[@LINE+1]]:{{[0-9]+}}: error:
opengpu.vtes.patch.read v10, x11, v12

# CHECK: :[[@LINE+1]]:{{[0-9]+}}: error:
opengpu.vtes.patch.read v10, v11, x12

# CHECK: :[[@LINE+1]]:{{[0-9]+}}: error:
opengpu.vtes.patch.read v32, v11, v12

# CHECK: :[[@LINE+1]]:{{[0-9]+}}: error:
opengpu.vtes.patch.read v10, v11, v12, v1.t

# CHECK: :[[@LINE+1]]:{{[0-9]+}}: error:
opengpu.vtes.patch.read v10, v11

# CHECK: :[[@LINE+1]]:{{[0-9]+}}: error:
opengpu.vtes.patch.read v10, v11, v12, v0.t, v0.t
