// RUN: %clang_cc1 -triple opengpu-unknown-none-elf -target-feature +v -O2 -emit-llvm -o - %s | FileCheck %s

#include <riscv_vector.h>

// CHECK-LABEL: define dso_local void @copy4(
// CHECK: call <vscale x 2 x i32> @llvm.riscv.vle.nxv2i32.p0.i32(
// CHECK: call void @llvm.riscv.vse.nxv2i32.p0.i32(
void copy4(unsigned *dst, const unsigned *src)
{
    vuint32m1_t value = __riscv_vle32_v_u32m1(src, 4);
    __riscv_vse32_v_u32m1(dst, value, 4);
}
