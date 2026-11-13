#!/usr/bin/env python3
"""Build an isolated macOS MLIR deadness library; never replace an installed tool."""
import argparse
import os
from pathlib import Path
import shlex
import subprocess
import sys


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--release", type=Path, required=True)
    parser.add_argument("--verilog", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--test-googletest", type=Path)
    args = parser.parse_args()
    if sys.platform != "darwin":
        parser.error("this overlay builder currently supports macOS only")
    release = args.release.resolve(strict=True)
    verilog = args.verilog.resolve(strict=True)
    output = args.output.absolute()
    output.mkdir(exist_ok=False, parents=True)
    source = Path(__file__).resolve().parents[1] / "lib/Interfaces/SideEffectInterfaces.cpp"
    library = output / "libMLIRSideEffectInterfaces.dylib"
    command = ["/usr/bin/clang++", "-dynamiclib", "-O3", "-DNDEBUG", "-std=c++17",
               "-fno-rtti", "-fno-exceptions", "-I" + str(release / "include"),
               str(source), "-L" + str(release / "lib"), "-lMLIRIR",
               "-lMLIRSupport", "-lLLVMSupport",
               "-Wl,-install_name,@rpath/libMLIRSideEffectInterfaces.dylib",
               "-Wl,-rpath," + str(release / "lib"),
               "-Wl,-compatibility_version,24.0.0",
               "-Wl,-current_version,24.0.0", "-o", str(library)]
    subprocess.run(command, check=True, env={**os.environ, "TMPDIR": str(output)})
    # The source is from the matching LLVM 24 interface. Refuse an ABI mismatch
    # rather than relying on a partially compatible replacement library.
    def exports(path):
        result = subprocess.check_output(["nm", "-gjU", str(path)], text=True)
        return {line for line in result.splitlines() if line.startswith(("__ZN4mlir", "__ZNK4mlir"))}
    original = release / "lib/libMLIRSideEffectInterfaces.dylib"
    if exports(library) != exports(original):
        raise RuntimeError("replacement MLIR interface export set differs")
    # Set the loader path only in this wrapper's child. Other tools/processes,
    # including already running generation, retain their original libraries.
    for name, tool in [("circt-verilog", verilog),
                       ("circt-opt", release / "bin/circt-opt")]:
        wrapper = output / name
        wrapper.write_text("#!/bin/sh\n" +
            "export DYLD_LIBRARY_PATH=" + shlex.quote(str(output)) + "\n" +
            "unset CIRCT_EXPERIMENTAL_LOCAL_CANONICALIZER CIRCT_EXPERIMENTAL_SKIP_CF_TRUTH\n" +
            "exec " + shlex.quote(str(tool)) + ' "$@"\n')
        wrapper.chmod(0o755)
    if args.test_googletest:
        googletest = args.test_googletest.resolve(strict=True)
        unit = output / "deadness-unit"
        test_source = source.parents[2] / "unittests/Interfaces/SideEffectInterfacesTest.cpp"
        command = ["/usr/bin/clang++", "-O1", "-DNDEBUG", "-fno-rtti",
                   "-DGTEST_HAS_RTTI=0", "-std=c++17",
                   "-I" + str(release / "include"),
                   "-I" + str(googletest / "include"), "-I" + str(googletest),
                   str(test_source), str(googletest / "src/gtest-all.cc"),
                   str(googletest / "src/gtest_main.cc"), "-L" + str(output),
                   "-L" + str(release / "lib"), "-lMLIRSideEffectInterfaces",
                   "-lMLIRArithDialect", "-lMLIRIR", "-lMLIRSupport",
                   "-lLLVMSupport", "-Wl,-rpath," + str(output),
                   "-Wl,-rpath," + str(release / "lib"), "-o", str(unit)]
        try:
            subprocess.run(command, check=True,
                           env={**os.environ, "TMPDIR": str(output)})
            subprocess.run([str(unit)], check=True,
                           env={**os.environ, "DYLD_LIBRARY_PATH": str(output)})
        finally:
            unit.unlink(missing_ok=True)
    print("isolated bounded-deadness tools:", output)


if __name__ == "__main__":
    main()
