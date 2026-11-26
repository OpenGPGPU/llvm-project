// RUN: %clang -target opengpu -mcpu=opengpu -O2 -S -o - %s | FileCheck %s --check-prefix=ASM
// RUN: %clang -target opengpu -mcpu=opengpu -O2 -c -o %t.o %s
// RUN: llvm-objdump --triple=opengpu --mcpu=opengpu -d %t.o | FileCheck %s --check-prefix=OBJ

// ASM-LABEL: patch_read_unmasked:
// ASM: opengpu.vtes.patch.read v10, v11, v12
void patch_read_unmasked(void) {
  __asm__ volatile("opengpu.vtes.patch.read v10, v11, v12"
                   ::: "v10", "v11", "v12", "memory");
}

// ASM-LABEL: patch_read_masked:
// ASM: opengpu.vtes.patch.read v10, v11, v12, v0.t
void patch_read_masked(void) {
  __asm__ volatile("opengpu.vtes.patch.read v10, v11, v12, v0.t"
                   ::: "v0", "v10", "v11", "v12", "memory");
}

// OBJ-LABEL: <patch_read_unmasked>:
// OBJ: opengpu.vtes.patch.read v10, v11, v12
// OBJ-LABEL: <patch_read_masked>:
// OBJ: opengpu.vtes.patch.read v10, v11, v12, v0.t
