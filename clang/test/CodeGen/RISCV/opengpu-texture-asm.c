// RUN: %clang -target opengpu -mcpu=opengpu -O2 -S -o - %s | FileCheck %s

// CHECK-LABEL: sample_texture_bits:
// CHECK: opengpu.tex.params.0
// CHECK: opengpu.tex.params.1
// CHECK: opengpu.tex.params.2
// CHECK: opengpu.tex.params.3
// CHECK: opengpu.tex.params.4
// CHECK: opengpu.tex.params.5
// CHECK: opengpu.tex.params.6
// CHECK: opengpu.tex.params.7
// CHECK: opengpu.tex.samplef.0
// CHECK: ret

unsigned sample_texture_bits(unsigned s, unsigned t, unsigned unit) {
  unsigned result;
  __asm__ volatile("opengpu.tex.params.0 %0, %1" : : "r"(s), "r"(t) : "memory");
#define PAIR(N) __asm__ volatile("opengpu.tex.params." #N " %0, %0" : : "r"(0u) : "memory")
  PAIR(1); PAIR(2); PAIR(3); PAIR(4); PAIR(5); PAIR(6); PAIR(7);
  __asm__ volatile("opengpu.tex.samplef.0 %0, %1, %2" : "=r"(result) : "r"(unit), "r"(1u) : "memory");
  return result;
}

// CHECK-LABEL: fetch_texture_bits:
// CHECK: opengpu.tex.params.0
// CHECK: opengpu.tex.params.1
// CHECK: opengpu.tex.params.2
// CHECK: opengpu.tex.params.3
// CHECK: opengpu.tex.params.4
// CHECK: opengpu.tex.params.5
// CHECK: opengpu.tex.params.6
// CHECK: opengpu.tex.params.7
// CHECK: opengpu.tex.fetchi.0
// CHECK: ret
unsigned fetch_texture_bits(int x, int y, unsigned unit) {
  unsigned result;
  __asm__ volatile("opengpu.tex.params.0 %0, %1" : : "r"(x), "r"(y) : "memory");
  PAIR(1); PAIR(2); PAIR(3); PAIR(4); PAIR(5); PAIR(6); PAIR(7);
  __asm__ volatile("opengpu.tex.fetchi.0 %0, %1, %2" : "=r"(result) : "r"(unit), "r"(0u) : "memory");
  return result;
}

// CHECK-LABEL: texture_width:
// CHECK: opengpu.tex.size.0
// CHECK: ret
int texture_width(unsigned unit, int lod) {
  int result;
  __asm__ volatile("opengpu.tex.size.0 %0, %1, %2" : "=r"(result) : "r"(unit), "r"(lod) : "memory");
  return result;
}
