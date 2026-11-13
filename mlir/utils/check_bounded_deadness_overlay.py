#!/usr/bin/env python3
"""Exercise isolated CIRCT tools through the real FlashSim parser/emitter/C++."""
import argparse
import os
from pathlib import Path
import subprocess
import sys
import tempfile

SV = r"""
module Top(input logic clock, reset, write, read_enable,
           input logic [1:0] index, input logic [7:0] data,
           output logic [7:0] q, peek, initialized);
  logic [7:0] words[0:3];
  initial initialized = 77;
  always @(posedge clock) begin
    if (reset) q <= 77; else if (read_enable) q <= words[index];
    if (reset) for (int i=0; i<4; i=i+1) words[i] <= i*3;
    else if (write) words[index] <= data;
  end
  assign peek = words[index];
  always @(posedge clock)
    if (!reset && write && data == 255) $fatal(1, "bad memory write");
endmodule
"""
MAIN = r'''
#include "dut.h"
#include <iostream>
#include <string>
int main(int argc, char **argv) {
 try {
  TopDut d;
  auto check = [&](unsigned q, unsigned peek) {
    d.poke_inputs(); d.eval_q(); d.eval_peek(); d.eval_initialized();
    if (d.q != q || d.peek != peek || d.initialized != 77) throw std::runtime_error("memory oracle");
    std::cout << unsigned(d.q) << ":" << unsigned(d.peek) << "\n";
  };
  d.clock=0; d.reset=0; d.write=0; d.read_enable=0; d.index=0; d.data=0;
  d.reset=1; d.tick(); check(77,0); d.reset=0;
  d.index=2; d.read_enable=1; d.tick(); check(6,6);
  d.write=1; d.data=42; d.tick(); check(6,42);
  d.write=0; d.read_enable=0; d.index=1; d.tick(); check(6,3);
  d.write=1; d.data=23; d.tick(); check(6,23);
  d.write=0; d.read_enable=1; d.tick(); check(23,23);
  d.index=2; d.tick(); check(42,42);
  d.reset=1; d.tick(); check(77,6);
  d.reset=0; d.tick(); check(6,6);
  if (argc>1 && std::string(argv[1])=="bad") {
    d.write=1; d.data=255; d.tick(); return 3;
  }
  return 0;
 } catch(const std::exception &error) { std::cerr << error.what(); return 42; }
}
'''


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--flashsim", type=Path, required=True)
    parser.add_argument("--original", type=Path, required=True)
    parser.add_argument("--bounded", type=Path, required=True)
    parser.add_argument("--temporary-parent", type=Path, required=True)
    parser.add_argument("--original-opt", type=Path)
    parser.add_argument("--bounded-opt", type=Path)
    args = parser.parse_args()
    if bool(args.original_opt) != bool(args.bounded_opt):
        parser.error("both --original-opt and --bounded-opt are required together")
    sys.path.insert(0, str(args.flashsim.resolve()))
    from flashsim.compile import compile_file
    observed = []
    with tempfile.TemporaryDirectory(prefix="deadness-", dir=args.temporary_parent) as raw:
        base = Path(raw)
        for name, tool in [("original", args.original), ("bounded", args.bounded)]:
            os.environ["CIRCT_VERILOG"] = str(tool.resolve())
            directory = base / name
            directory.mkdir()
            source = directory / "Top.sv"
            source.write_text(SV)
            model = compile_file(source, directory / "dut.h", frontend="circt", assertions=True)
            assert len(model.assertions) == 1
            (directory / "check.cpp").write_text(MAIN)
            binary = directory / "check"
            subprocess.run(["/usr/bin/clang++", "-std=c++17", "-O1", "-fsanitize=address,undefined",
                str(directory / "check.cpp"), *map(str, sorted(directory.glob("dut_*.cpp"))),
                "-o", str(binary)], check=True)
            healthy = subprocess.run([str(binary)], text=True, capture_output=True)
            assert healthy.returncode == 0, (name, healthy.stderr)
            fault = subprocess.run([str(binary), "bad"], text=True, capture_output=True)
            assert fault.returncode == 42 and "bad memory write" in fault.stderr, (name, fault)
            observed.append(healthy.stdout)
        assert observed[0] == observed[1]
        if args.original_opt:
            for depth in (32, 64, 128, 256):
                source = base / "Deep.sv"
                moore = base / "Deep.mlir"
                source.write_text("\n".join(
                    f"module Deep{module}(input logic[{depth-1}:0] c,"
                    "input logic[31:0] x,output logic[31:0] y);"
                    "always_comb begin y=0;" +
                    "".join(f"if(c[{index}]) begin " for index in range(depth)) +
                    "y=x;" + "end " * depth + "end endmodule"
                    for module in range(8)))
                subprocess.run([str(args.bounded.resolve()), "--single-unit",
                    "--import-only", str(source), "-o", str(moore)],
                    check=True, capture_output=True)
                results = []
                for name, tool in [("original", args.original_opt),
                                   ("bounded", args.bounded_opt)]:
                    result = subprocess.run([str(tool.resolve()),
                        "--mlir-disable-threading",
                        "--pass-pipeline=builtin.module(canonicalize)",
                        "--mlir-timing", str(moore)],
                        check=True, text=True, capture_output=True)
                    results.append(result.stdout)
                    timing = next(line.strip() for line in result.stderr.splitlines()
                                  if "CanonicalizerPass" in line)
                    print(depth, name, timing)
                assert results[0] == results[1], (depth, "different canonical IR")
    print("original/bounded actual C++ initial/reset/sync-read/old-data/hold/fault equivalence PASS")


if __name__ == "__main__":
    main()
