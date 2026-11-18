#!/usr/bin/env python3
"""Build an isolated macOS MLIR analysis library against a matching LLVM SDK."""
import argparse
import hashlib
import os
from pathlib import Path
import re
import subprocess
import sys


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--release", type=Path, required=True)
    parser.add_argument("--sdk-source", type=Path, required=True,
                        help="LLVM source matching the release; other analyses are unchanged")
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    if sys.platform != "darwin":
        parser.error("this isolated shared-library builder supports macOS only")
    release = args.release.resolve(strict=True)
    source = args.sdk_source.resolve(strict=True) / "mlir/lib/Analysis"
    replacement = Path(__file__).resolve().parents[1] / "lib/Analysis/Liveness.cpp"
    text = (source / "CMakeLists.txt").read_text()
    body = text.split("add_mlir_library(MLIRAnalysis", 1)[1]
    units = body.split("ADDITIONAL_HEADER_DIRS", 1)[0].split()
    if not units or any(not name.endswith(".cpp") for name in units) or "Liveness.cpp" not in units:
        raise RuntimeError("unrecognized SDK analysis source list")
    libraries = body.split("LINK_LIBS PUBLIC", 1)[1].split(")", 1)[0].split()
    paths = [replacement if name == "Liveness.cpp" else source / name for name in units]
    bound = [*paths, *source.rglob("*.h")]
    def fingerprints():
        return {str(p): hashlib.sha256(p.read_bytes()).hexdigest() for p in bound}
    before = fingerprints()
    output = args.output.absolute()
    output.mkdir(exist_ok=False, parents=True)
    library = output / "libMLIRAnalysis.dylib"
    command = ["/usr/bin/clang++", "-dynamiclib", "-O3", "-DNDEBUG", "-std=c++17",
               "-fno-rtti", "-fno-exceptions", "-I" + str(release / "include"),
               *map(str, paths), "-L" + str(release / "lib"),
               *["-l" + name for name in dict.fromkeys(
                   [*libraries, "MLIRIR", "MLIRSupport", "LLVMSupport"])],
               "-Wl,-install_name,@rpath/libMLIRAnalysis.dylib",
               "-Wl,-rpath," + str(release / "lib"),
               "-Wl,-compatibility_version,24.0.0",
               "-Wl,-current_version,24.0.0", "-o", str(library)]
    try:
        subprocess.run(command, check=True, env={**os.environ, "TMPDIR": str(output)})
        def exports(path):
            symbols = subprocess.check_output(["nm", "-gjU", str(path)], text=True)
            return {s for s in symbols.splitlines()
                    if s.startswith(("__ZN4mlir", "__ZNK4mlir"))}
        missing = exports(release / "lib/libMLIRAnalysis.dylib") - exports(library)
        if missing:
            raise RuntimeError("replacement analysis library lacks SDK exports: " + repr(sorted(missing)))
        if before != fingerprints():
            raise RuntimeError("analysis sources changed during compilation")
    except BaseException:
        library.unlink(missing_ok=True)
        raise
    print("isolated liveness analysis library:", library)


if __name__ == "__main__":
    main()
