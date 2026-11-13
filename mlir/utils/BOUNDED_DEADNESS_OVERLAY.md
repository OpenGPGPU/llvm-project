# Isolated bounded MLIR deadness query

This source keeps the complete CIRCT pass pipeline. The only deadness change is
an upper bound of 64 work items, counting visited intervals and enqueued blocks.
Exhausting the budget returns `false`: the optimization retains the operation.
Memory-effect classification, terminator/symbol protection, and the public
`isOpTriviallyDead` use-empty requirement remain intact. No assertion, memory,
state, module, or hardware capacity is removed to accelerate compilation.

The library overlay is a macOS prototype for the matching LLVM 24 shared release.
The builder compiles the complete interface implementation and requires its MLIR
export set to match the original library. It never edits an installed compiler
or library. The debug wrappers only select the new library in their child.
For guarded generation, use the original real compiler executable and an explicit
`DYLD_LIBRARY_PATH`; do not pass a shell wrapper as `CIRCT_VERILOG`.

## Reproduce

Use a new output inside the task workspace. `RELEASE` is the matching full shared
CIRCT release, `VERILOG` the real CIRCT compiler, `GTEST` the googletest source,
and `FLASHSIM` the actual FlashSim source checkout.

```sh
python3 mlir/utils/build_bounded_deadness_overlay.py \
  --release "$RELEASE" --verilog "$VERILOG" --output "$OUT" \
  --test-googletest "$GTEST"
python3 mlir/utils/check_bounded_deadness_overlay.py \
  --flashsim "$FLASHSIM" --original "$VERILOG" \
  --bounded "$OUT/circt-verilog" --temporary-parent "$TASK_TMP" \
  --original-opt "$RELEASE/bin/circt-opt" --bounded-opt "$OUT/circt-opt"
CIRCT_VERILOG="$OUT/circt-verilog" CIRCT_OPT="$OUT/circt-opt" \
  PYTHONDONTWRITEBYTECODE=1 TMPDIR="$TASK_TMP" python3 -m pytest \
  -q -p no:cacheprovider --basetemp="$TASK_TMP/pytest"
```

Run pytest from the FlashSim checkout. The semantic checker removes its generated
C++, binaries, RTL, and MLIR in `finally`; remove pytest temporary output after
completion. Preserve the overlay only while a consumer still needs it.

## Results and limits

All 349 current FlashSim tests pass with the isolated library (no skips). Six
actual library interface tests pass for small pure/effectful regions, budget
boundaries in wide and long regions, and live pure values. Original and bounded
compilers produce C++ that passes the same independent synchronous-memory oracle:
read-before-write, read-enable hold, non-destructive access, reset, initial constant,
and a real write fault. Existing bound-assertion tests also check both instance
faults and pre-NBA sampling.

Eight depth-256 nested Moore conditionals produce identical canonical IR; the
single canonicalizer measured 1.8587 seconds with the original library and 0.5823
seconds with the bound. This is a small compiler experiment, not a complete GPU
speed measurement. The full GPU import, source guards, assertion-enabled generated
model, and actual DUT execution remain required.

Existing frontend rejection of an initial process and a subsequent independent
writer to the same signal or memory is unchanged. The checker uses an initial
constant and properly reset mutable memory; it does not claim new multi-driver
support. A conservative query can leave more IR alive; larger-model memory use
and later-pass compatibility must be measured before adopting the overlay.
