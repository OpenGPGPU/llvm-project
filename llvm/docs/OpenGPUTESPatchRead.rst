OpenGPU TES owned patch-read MC support
======================================

Scope and encoding
------------------

``opengpu.vtes.patch.read vd, vs1, vs2[, v0.t]`` declares one MC-only
OpenGPU instruction under the existing XOpengpu predicate. All three operands
are vector registers. It uses CUSTOM1 opcode ``0x2b``, function 33 in bits
31:26, funct3=0, and the normal vector operand fields. Bit 25 is vm: 1 for
unmasked, 0 for the optional ``v0.t`` form. CUSTOM0 geometry function 33 remains
a separate scalar output-publication operation; this declaration grants no
geometry-stage authority.

The instruction reads the retained TES task's output patch, per-patch words or
six levels. ``vs1`` contains full-width vertex indices; ``vs2`` contains the
output/per-patch/level descriptor. The actual owned service checks every
selected lane against its captured patch layout. Masked execution preserves
inactive old-vd values. Encoding support does not validate layout, VL, defined
operands, old destination, instruction ordering or task ownership. Those are
program-validator and runtime-provider contracts. A program-v2 permission is
not a new public LLVM feature flag.

The declaration has ``hasSideEffects=1``, ``mayLoad=1``, ``mayStore=0``,
``isConvergent=1`` and ``isNotDuplicable=1``, with the existing format's VL/VTYPE
uses. This context-dependent, fault-capable read cannot be treated as a pure
load or duplicated across divergent/task lifetime boundaries. No intrinsic,
automatic instruction selection or generic memory lowering is added. Future
codegen must model actual owned resource/context dependencies across task
bind, finish and cancellation; these MC properties alone do not supply them.

Production definitions and bounded checks
-----------------------------------------

The shared mask operands, bare-symbol operand and OpenGPU vector format are
moved without changing their definitions. Expanding their production includes
reproduces the three original files byte for byte, in the same declaration
order and lexical scopes; each definition occurs once. The small TableGen test
includes the actual features, system operands, register classes, formats, mask
operands and TES declaration. It does not shadow any of those types.

Records, asm-matcher, code-emitter and disassembler generation pass strict
FileCheck against this same production instruction. Checks cover exact fixed
bits, vector/mask operand fields, the existing feature predicate, VL/VTYPE and
side-effect properties. Four independently mutated declarations (wrong
function, CUSTOM0 opcode, pure read, non-mask operand) fail the record checks.
Each generator ran serially with a 180-second limit and a 512MiB RSS guard;
observed peak RSS stayed below 104MiB. All generated output was removed.

From the LLVM source root, with a compatible existing ``llvm-tblgen`` and
``FileCheck`` on PATH, reproduce the four small-target checks serially::

  mkdir -p .artifacts
  P40_TD_TMP=$(mktemp -d "$PWD/.artifacts/tes-patch-XXXXXX")
  P40_TD_TEST=llvm/test/TableGen/riscv-opengpu-tes-patch.td
  P40_TD_STATUS=0
  for P40_TD_PAIR in print-records:RECORD gen-asm-matcher:MATCH gen-emitter:EMIT gen-disassembler:DECODE; do
    P40_TD_MODE=${P40_TD_PAIR%:*}
    P40_TD_PREFIX=${P40_TD_PAIR#*:}
    llvm-tblgen -"$P40_TD_MODE" -I llvm/include -I llvm/lib/Target/RISCV \
      "$P40_TD_TEST" -o "$P40_TD_TMP/result" && \
      FileCheck "$P40_TD_TEST" --check-prefix="$P40_TD_PREFIX" \
        --input-file="$P40_TD_TMP/result" || { P40_TD_STATUS=1; break; }
  done
  # After the last generator exits and no consumer needs this private output:
  rm -r -- "$P40_TD_TMP"
  test "$P40_TD_STATUS" -eq 0

Use a process supervisor or container to retain the same wall/RSS limits. The
small-target test's RUN lines also describe the standard TableGen test lane.

Unverified integration and remaining ISA work
--------------------------------------------

The available LLVM 24.0.0git tools came from an existing read-only build whose
recorded revision is older than this source baseline. They consumed the new
TD directly as generators; no new target-specific MC executable was built.
The full RISCV.td expansion exceeded the 512MiB guard and was terminated.
Full-target generation and the new instruction's actual assembler/object/
disassembler roundtrip, feature-off and malformed-operand MC tests remain
pending against rebuilt matching tools. The committed MC tests specify those
checks; generation alone does not count as passing them.

A safe future MC-only tool build must regenerate all instruction-ID-dependent
MC tables and rebuild the coherent RISCV parser/descriptor/disassembler closure;
linking a new table into old target objects can mix instruction enum IDs.
No existing LLVM build, primary branch or other worktree was modified here.

This single instruction does not close P26/P40 or the whole OpenGPU ISA:

* TCS CUSTOM1 functions 20/21/22 and the separate CUSTOM0 geometry/TES
  publication family still lack MC declarations in this baseline.
* Full TCS/TES/GS lowering, intrinsic and scheduling/resource dependencies are
  separate work. There is no automatic LLVM emission of this instruction.
* Image and other resource-service instructions/permissions are not supplied
  by the existing typed texture MC family or by this patch-read declaration.
* Actual task-owned binding, initialization/isolation, fault/cancel/retirement
  and complete CU/host execution still need matching provider qualification.

No shader execution, GPU result, full LLVM build, Mesa rebuild, JVM or RTL
qualification was performed by these MC-only checks.
