#!/usr/bin/env python3
"""Compare actual SDK liveness analysis on fixed and seeded cyclic SSA graphs."""
import argparse
import json
import os
from pathlib import Path
import random
import statistics
import subprocess
import tempfile


def graph(seed, blocks=None, arguments=16):
    randomizer = random.Random(seed)
    blocks = 12 + seed % 12 if blocks is None else blocks
    lines = ["func.func @case_%d(%%condition: i1, %s) -> i32 {" %
             (seed, ", ".join("%%arg%d: i32" % n for n in range(arguments))),
             "  cf.br ^b0(%arg0, %arg1 : i32, i32)"]
    for index in range(blocks):
        argument = randomizer.randrange(arguments)
        lines.append("^b%d(%%p%d: i32, %%q%d: i32):" % (index, index, index))
        if index % 3 == 0:
            lines.extend([
                "  %%v%d = scf.execute_region -> i32 {" % index,
                "    %%nested%d = arith.addi %%p%d, %%arg%d : i32" %
                (index, index, argument),
                "    scf.yield %%nested%d : i32" % index, "  }"])
        else:
            lines.append("  %%v%d = arith.addi %%p%d, %%arg%d : i32" %
                         (index, index, argument))
        lines.append("  %%w%d = arith.xori %%v%d, %%q%d : i32" % (index, index, index))
        def target(number):
            return "^exit" if number == blocks else "^b%d" % number
        lines.append("  cf.cond_br %%condition, %s(%%w%d, %%v%d : i32, i32), "
                     "%s(%%v%d, %%arg%d : i32, i32)" %
                     (target(index + 1), index, index,
                      target(randomizer.randrange(blocks + 1)), index, argument))
    lines.extend(["^exit(%left: i32, %right: i32):",
                  "  %sum = arith.addi %left, %right : i32", "  return %sum : i32",
                  "^unreachable:", "  %zero = arith.constant 0 : i32",
                  "  return %zero : i32", "}"])
    return "\n".join(lines)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--release", type=Path, required=True)
    parser.add_argument("--overlay", type=Path, required=True)
    parser.add_argument("--temporary-parent", type=Path, required=True)
    parser.add_argument("--benchmark-blocks", type=int, default=0)
    parser.add_argument("--benchmark-arguments", type=int, default=256)
    args = parser.parse_args()
    if not 0 <= args.benchmark_blocks <= 10000 or not 2 <= args.benchmark_arguments <= 4096:
        parser.error("bounded benchmark requires 0..10000 blocks and 2..4096 arguments")
    release = args.release.resolve(strict=True)
    overlay = args.overlay.resolve(strict=True)
    tool, checker = release / "bin/mlir-opt", release / "bin/FileCheck"
    baseline = {k: v for k, v in os.environ.items() if not k.startswith("DYLD_")}
    changed = {**baseline, "DYLD_LIBRARY_PATH": str(overlay)}
    for environment, expected in [(baseline, release / "lib/libMLIRAnalysis.dylib"),
                                  (changed, overlay / "libMLIRAnalysis.dylib")]:
        loaded = subprocess.run([str(tool), "--version"], check=True,
                                env={**environment, "DYLD_PRINT_LIBRARIES": "1"},
                                stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        libraries = [line for line in loaded.stderr.splitlines()
                     if " /" in line and line.endswith("/libMLIRAnalysis.dylib")]
        if len(libraries) != 1 or str(expected) not in libraries[0]:
            raise RuntimeError("unexpected analysis library provider: " + repr(libraries))

    def analyze(path, environment):
        return subprocess.run([str(tool), str(path), "--mlir-disable-threading",
                               "--pass-pipeline=builtin.module(func.func(test-print-liveness))",
                               "--split-input-file"], env=environment, check=True,
                              stdout=subprocess.PIPE, stderr=subprocess.STDOUT).stdout

    original = Path(__file__).resolve().parents[1] / "test/Analysis/test-liveness.mlir"
    before, after = analyze(original, baseline), analyze(original, changed)
    for output in (before, after):
        subprocess.run([str(checker), str(original)], input=output, check=True)
    if before != after:
        raise RuntimeError("existing liveness regression output changed")
    with tempfile.TemporaryDirectory(prefix="liveness-", dir=args.temporary_parent) as directory:
        source = Path(directory) / "graphs.mlir"
        source.write_text("\n// -----\n".join(graph(seed) for seed in range(64)))
        if analyze(source, baseline) != analyze(source, changed):
            raise RuntimeError("cyclic/nested SSA liveness differs from the SDK")
        if args.benchmark_blocks:
            binary = Path(directory) / "liveness-benchmark"
            benchmark = Path(__file__).with_name("LivenessBenchmark.cpp")
            command = ["/usr/bin/clang++", "-O3", "-DNDEBUG", "-std=c++17",
                       "-fno-rtti", "-fno-exceptions", "-I" + str(release / "include"),
                       str(benchmark), "-L" + str(release / "lib"),
                       *["-l" + name for name in ("MLIRParser", "MLIRAnalysis",
                           "MLIRArithDialect", "MLIRControlFlowDialect", "MLIRFuncDialect",
                           "MLIRSCFDialect", "MLIRIR", "MLIRSupport", "LLVMSupport")],
                       "-Wl,-rpath," + str(release / "lib"), "-o", str(binary)]
            subprocess.run(command, check=True, env=baseline)
            source.write_text(graph(73, args.benchmark_blocks, args.benchmark_arguments))
            rows = {"sdk": [], "changed": []}
            signature = None
            for trial in range(3):
                order = [("sdk", baseline), ("changed", changed)]
                if trial % 2:
                    order.reverse()
                for name, environment in order:
                    row = json.loads(subprocess.check_output(
                        [str(binary), str(source), "3"], env=environment, text=True))
                    current = {key: value for key, value in row.items() if key != "analysisSeconds"}
                    if signature is not None and signature != current:
                        raise RuntimeError("benchmark liveness set counts changed")
                    signature = current
                    rows[name].append(row["analysisSeconds"])
            print("LIVENESS_BENCHMARK " + json.dumps({
                "scope": "synthetic analysis construction only; excludes parsing, printing and destruction",
                "arguments": args.benchmark_arguments, **signature,
                "sdkMedianSeconds": statistics.median(rows["sdk"]),
                "changedMedianSeconds": statistics.median(rows["changed"])}))
    print("LIVENESS_EQUIVALENCE_PASS: original FileCheck suite and 64 cyclic/nested SSA graphs; "
          "actual SDK/new library providers verified, byte-identical complete analysis output")


if __name__ == "__main__":
    main()
