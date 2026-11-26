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

Actual MC integration and remaining ISA work
-------------------------------------------

The original 512MiB full-RISCV expansion failure remains a failure at that
limit. A subsequent private build regenerated all ten consumed RISC-V tables
from current production source: matcher, writer, compressed emitter,
disassembler, instruction info, code emitter, register info (including its
split outputs), searchable tables, subtarget info and MC options. All sixteen
AsmParser/Disassembler/MCTargetDesc/TargetInfo translation units were rebuilt
without a PCH against these tables and current headers. No old RISC-V target
object or archive was linked. Separate ``llvm-mc`` and ``llvm-objdump`` binaries
linked successfully with the existing target-independent LLVM 24 libraries
and tool frontend objects; this is a coherent MC-target build, not a complete
LLVM rebuild. The primary LLVM checkout and shared build were read-only.

The serial build completed in 46.57s under a 180-second per-command and
1800-second overall deadline, with a 2048MiB owned-process-group RSS guard.
Largest sampled group RSS was 804272KiB, not an absolute physical-memory peak.
All 2810 guarded source/configuration/tool/library inputs matched before and
after. The sixteen actual MMD files named 353 unique dependencies, each either
in that guard or freshly generated in the private output. The compiler used
``-O0``/no PCH for this local MC qualification; source instruction properties
and encoding were unchanged.

All eleven ``xopengpu-*`` MC files passed all 21 original RUN pipelines against
these freshly linked tools, with strict FileCheck and pipefail. This includes
TES patch-read assembly, original object/disassembly roundtrips, extension-off
rejection, malformed registers/masks/arity and reserved encodings; existing
SIMT, base and texture MC cases passed too. The new scalar language-info cases
below passed the same actual paths. The 2821 build plus MC input files remained
unchanged. Original multiline RUN directives were joined before execution;
no source requirement or test assertion was relaxed.

To reproduce with a configured source-matching LLVM build, build its
``llvm-mc`` and ``llvm-objdump`` targets serially; do not mix new instruction
IDs with old RISC-V objects. For the bounded local build above, extract the
ten standard generator commands and all sixteen MC compiler commands from
``build.ninja``/``compile_commands.json``. Redirect only their source and output
paths to the current checkout and a private directory, put private generated
headers first, remove shared PCH flags and use ``-O0``. Regenerate MCOptions
with the configured ``llvm-min-tblgen``. Relink the two tool frontends with all
sixteen new objects and only the original target-independent libraries.
Retain the input/MMD and owned-process-group guards described above.

A complete production Clang/LLVM 24 build has also passed from this source.
The Release build enabled Clang and the RISCV target with assertions and RTTI
disabled. It built ``clang``, ``llc``, ``llvm-mc``, ``llvm-objdump``,
``llvm-objcopy``, ``clang-offload-packager``, ``FileCheck``, ``not``,
``llvm-config``, ``count`` and ``split-file`` in 2,386.87 seconds with 3.66 GiB
peak charged memory. The focused lit lane passed all 14 discovered tests: the
production TableGen check, every ``xopengpu-*`` MC file, the existing Clang
texture inline-assembly test and the new masked/unmasked TES patch-read Clang
inline-assembly test. Reproduce with the normal CMake/Ninja LLVM build and run
``llvm-lit`` over those files with the matching build's tools on PATH.

The focused actual MC commands, with those matching tools on PATH, are::

  for MC_CASE in tes-patch tess-info; do
    MC_TEST=llvm/test/MC/RISCV/xopengpu-$MC_CASE.s
    llvm-mc "$MC_TEST" -triple=opengpu -mcpu=opengpu -M no-aliases -show-encoding | FileCheck "$MC_TEST" --check-prefixes=INST,ENC
    llvm-mc "$MC_TEST" -filetype=obj -triple=opengpu -mcpu=opengpu | llvm-objdump --triple=opengpu --mcpu=opengpu -M no-aliases -d - | FileCheck "$MC_TEST" --check-prefix=OBJ
    not llvm-mc "$MC_TEST" -triple=riscv32 -mattr=-xopengpu 2>&1 | FileCheck "$MC_TEST" --check-prefix=NOEXT
    MC_BAD=llvm/test/MC/RISCV/xopengpu-$MC_CASE-invalid.s
    not llvm-mc "$MC_BAD" -triple=opengpu -mcpu=opengpu 2>&1 | FileCheck "$MC_BAD"
    MC_RESERVED=llvm/test/MC/RISCV/xopengpu-$MC_CASE-reserved.txt
    llvm-mc --disassemble -triple=opengpu -mcpu=opengpu < "$MC_RESERVED" 2>&1 | FileCheck "$MC_RESERVED"
  done

Run these pipelines with ``set -e -o pipefail`` and the normal LLVM test
``not`` tool; a zero-test selection is not a pass. All ended private tables,
objects, binaries, MMD files and logs are removed after consumers exit.

Independent scalar language info
--------------------------------

``opengpu.tess.info rd, rs1`` declares CUSTOM0 function37 (bits31:25),
funct3=0 and reserved rs2=0, using ordinary scalar register operands. The
scalar value in rs1 is the selector, not an assembler immediate. Selector6
reads the independently bound original input-patch PrimitiveID. TES selectors
0..5 keep their existing geometry-owner meanings; TCS permits only selector6
under its separate language profile. Stage/profile/binding/warp validity and
cancellation are runtime/validator contracts, not MC predicates. This encoding
does not authorize any other geometry operation.

The context read is side-effecting, convergent and not duplicable, with neither
a generic memory load nor a store. No intrinsic or automatic selection is
added. Actual scalar-register boundary encodings, object roundtrips,
extension-off errors, vector/immediate/wrong-arity errors and nonzero reserved
rs2/funct3 rejection passed with the new matching MC tools.

These MC declarations do not close P26/P40 or the whole OpenGPU ISA:

* TCS CUSTOM1 functions 20/21/22 and the separate CUSTOM0 geometry/TES
  publication family still lack MC declarations in this baseline.
* Full TCS/TES/GS lowering, intrinsic and scheduling/resource dependencies are
  separate work. There is no automatic LLVM emission of this instruction.
* Image and other resource-service instructions/permissions are not supplied
  by the existing typed texture MC family or by this patch-read declaration.
* Actual task-owned binding, initialization/isolation, fault/cancel/retirement
  and complete CU/host execution still need matching provider qualification.

The complete compiler build and focused MC/Clang tests do not perform shader
execution, GPU result validation, Mesa rebuild, JVM or RTL qualification.
Automatic TCS/TES/GS lowering and complete P39/P40 acceptance remain separate.
