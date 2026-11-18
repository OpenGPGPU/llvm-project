//===- LivenessBenchmark.cpp - Bounded analysis timing ---------------------===//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "mlir/Analysis/Liveness.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/ControlFlow/IR/ControlFlowOps.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/Parser/Parser.h"
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <vector>

int main(int argc, char **argv) {
  if (argc != 3)
    return 2;
  unsigned repeats = std::atoi(argv[2]);
  if (!repeats || repeats > 11)
    return 2;
  mlir::MLIRContext context;
  context.disableMultithreading();
  context.loadDialect<mlir::arith::ArithDialect, mlir::cf::ControlFlowDialect,
                      mlir::func::FuncDialect, mlir::scf::SCFDialect>();
  auto module = mlir::parseSourceFile<mlir::ModuleOp>(argv[1], &context);
  if (!module)
    return 1;
  using Clock = std::chrono::steady_clock;
  std::vector<double> times;
  uint64_t blocks = 0, liveIn = 0, liveOut = 0;
  for (unsigned repeat = 0; repeat < repeats; ++repeat) {
    auto start = Clock::now();
    mlir::Liveness liveness(module->getOperation());
    times.push_back(std::chrono::duration<double>(Clock::now() - start).count());
    blocks = liveIn = liveOut = 0;
    module->walk([&](mlir::Block *block) {
      ++blocks;
      liveIn += liveness.getLiveIn(block).size();
      liveOut += liveness.getLiveOut(block).size();
    });
  }
  std::sort(times.begin(), times.end());
  std::cout << std::setprecision(12) << "{\"analysisSeconds\":"
            << times[times.size() / 2] << ",\"blocks\":" << blocks
            << ",\"liveIn\":" << liveIn << ",\"liveOut\":" << liveOut
            << "}\n";
}
