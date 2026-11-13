//===- SideEffectInterfacesTest.cpp - Unit tests for Resource hierarchy ---===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/Dialect.h"
#include "mlir/Interfaces/SideEffectInterfaces.h"
#include "mlir/Support/TypeID.h"
#include "llvm/Support/Casting.h"
#include <gtest/gtest.h>

using namespace mlir;
using namespace mlir::SideEffects;

namespace {

struct RecursiveEffectsOp
    : Op<RecursiveEffectsOp, OpTrait::HasRecursiveMemoryEffects> {
  using Op::Op;
  static StringRef getOperationName() { return "effecttest.recursive"; }
  static ArrayRef<StringRef> getAttributeNames() { return {}; }
};

struct EffectTestDialect : Dialect {
  explicit EffectTestDialect(MLIRContext *ctx)
      : Dialect(getDialectNamespace(), ctx, TypeID::get<EffectTestDialect>()) {
    addOperations<RecursiveEffectsOp>();
    allowUnknownOperations();
  }
  static StringRef getDialectNamespace() { return "effecttest"; }
};

struct DeadOpFixture : testing::Test {
  MLIRContext context;
  Location loc = UnknownLoc::get(&context);
  OwningOpRef<ModuleOp> module;

  void SetUp() override {
    context.getOrLoadDialect<EffectTestDialect>();
    context.getOrLoadDialect<arith::ArithDialect>();
    module = ModuleOp::create(loc);
  }

  Operation *makeRecursive(unsigned blocks, unsigned pureOpsPerBlock,
                           bool unknownEffect = false) {
    OpBuilder builder = OpBuilder::atBlockEnd(module->getBody());
    OperationState state(loc, RecursiveEffectsOp::getOperationName());
    state.addRegion();
    Operation *op = builder.create(state);
    for (unsigned i = 0; i < blocks; ++i) {
      auto *block = new Block();
      op->getRegion(0).push_back(block);
      OpBuilder nested = OpBuilder::atBlockEnd(block);
      for (unsigned j = 0; j < pureOpsPerBlock; ++j)
        arith::ConstantIntOp::create(nested, loc, 0, 1);
      if (unknownEffect && i == 0) {
        OperationState unknown(loc, "effecttest.unknown");
        nested.create(unknown);
      }
    }
    return op;
  }
};

/// Custom resource hierarchy (root -> child -> grandchild) for testing.
struct TestRootResource : public Resource::Base<TestRootResource> {
  MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(TestRootResource)
  TestRootResource() = default;
  StringRef getName() const override { return "TestRoot"; }

protected:
  TestRootResource(TypeID id) : Base(id) {}
};

struct TestChildResource
    : public Resource::Base<TestChildResource, TestRootResource> {
  MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(TestChildResource)
  TestChildResource() = default;
  StringRef getName() const override { return "TestChild"; }
  Resource *getParent() const override { return TestRootResource::get(); }

protected:
  TestChildResource(TypeID id) : Base(id) {}
};

struct TestGrandchildResource
    : public Resource::Base<TestGrandchildResource, TestChildResource> {
  MLIR_DEFINE_EXPLICIT_INTERNAL_INLINE_TYPE_ID(TestGrandchildResource)
  StringRef getName() const override { return "TestGrandchild"; }
  Resource *getParent() const override { return TestChildResource::get(); }
};

} // namespace

TEST_F(DeadOpFixture, SmallPureAndEffectfulRegions) {
  EXPECT_TRUE(wouldOpBeTriviallyDead(makeRecursive(1, 2)));
  EXPECT_FALSE(wouldOpBeTriviallyDead(makeRecursive(1, 2, true)));
}

TEST_F(DeadOpFixture, UsedPureValueRemainsLive) {
  OpBuilder builder = OpBuilder::atBlockEnd(module->getBody());
  auto value = arith::ConstantIntOp::create(builder, loc, 7, 32);
  arith::AddIOp::create(builder, loc, value, value);
  EXPECT_TRUE(wouldOpBeTriviallyDead(value));
  EXPECT_FALSE(isOpTriviallyDead(value));
}

TEST_F(DeadOpFixture, BoundedWideAndLongRegionsRemainConservative) {
  EXPECT_TRUE(wouldOpBeTriviallyDead(makeRecursive(1, 60)));
  EXPECT_FALSE(wouldOpBeTriviallyDead(makeRecursive(1, 61)));
  EXPECT_TRUE(wouldOpBeTriviallyDead(makeRecursive(31, 0)));
  EXPECT_FALSE(wouldOpBeTriviallyDead(makeRecursive(32, 0)));
  EXPECT_FALSE(wouldOpBeTriviallyDead(makeRecursive(1, 70)));
  EXPECT_FALSE(wouldOpBeTriviallyDead(makeRecursive(70, 0)));
  EXPECT_FALSE(wouldOpBeTriviallyDead(makeRecursive(70, 1, true)));
}

TEST(SideEffectResourceTest, BuiltInHierarchyIsaCast) {
  EXPECT_TRUE(isa<DefaultResource>(DefaultResource::get()));
  EXPECT_TRUE(isa<DefaultResource>(AutomaticAllocationScopeResource::get()));
  EXPECT_FALSE(isa<AutomaticAllocationScopeResource>(DefaultResource::get()));

  // Each type has its own singleton; cast yields the child object as parent*
  EXPECT_NE(cast<DefaultResource>(AutomaticAllocationScopeResource::get()),
            nullptr);
  EXPECT_TRUE(isa<DefaultResource>(
      cast<DefaultResource>(AutomaticAllocationScopeResource::get())));

  EXPECT_NE(dyn_cast<DefaultResource>(AutomaticAllocationScopeResource::get()),
            nullptr);
  EXPECT_EQ(dyn_cast<AutomaticAllocationScopeResource>(DefaultResource::get()),
            nullptr);
}

TEST(SideEffectResourceTest, CustomHierarchyIsaCast) {
  // Root and child
  EXPECT_TRUE(isa<TestRootResource>(TestRootResource::get()));
  EXPECT_TRUE(isa<TestRootResource>(TestChildResource::get()));
  EXPECT_FALSE(isa<TestChildResource>(TestRootResource::get()));
  EXPECT_NE(cast<TestRootResource>(TestChildResource::get()), nullptr);

  // Grandchild isa root, child, and self
  EXPECT_TRUE(isa<TestRootResource>(TestGrandchildResource::get()));
  EXPECT_TRUE(isa<TestChildResource>(TestGrandchildResource::get()));
  EXPECT_TRUE(isa<TestGrandchildResource>(TestGrandchildResource::get()));

  // Root/child are not isa grandchild
  EXPECT_FALSE(isa<TestGrandchildResource>(TestRootResource::get()));
  EXPECT_FALSE(isa<TestGrandchildResource>(TestChildResource::get()));

  // Cast grandchild to root and child (each type has its own singleton)
  EXPECT_NE(cast<TestRootResource>(TestGrandchildResource::get()), nullptr);
  EXPECT_NE(cast<TestChildResource>(TestGrandchildResource::get()), nullptr);

  // dyn_cast
  EXPECT_EQ(dyn_cast<TestGrandchildResource>(TestRootResource::get()), nullptr);
  EXPECT_NE(dyn_cast<TestRootResource>(TestGrandchildResource::get()), nullptr);

  // getParent chain
  EXPECT_EQ(TestGrandchildResource::get()->getParent(),
            TestChildResource::get());
  EXPECT_EQ(TestChildResource::get()->getParent(), TestRootResource::get());
  EXPECT_EQ(TestRootResource::get()->getParent(), DefaultResource::get());
  EXPECT_EQ(DefaultResource::get()->getParent(), nullptr);

  // isSubresourceOf
  EXPECT_TRUE(
      TestGrandchildResource::get()->isSubresourceOf(TestRootResource::get()));
  EXPECT_TRUE(
      TestGrandchildResource::get()->isSubresourceOf(TestChildResource::get()));
  EXPECT_FALSE(
      TestRootResource::get()->isSubresourceOf(TestGrandchildResource::get()));

  // Custom hierarchy is a subresource of DefaultResource (the root).
  EXPECT_FALSE(TestRootResource::get()->isDisjointFrom(DefaultResource::get()));
  EXPECT_TRUE(TestRootResource::get()->isSubresourceOf(DefaultResource::get()));
  EXPECT_TRUE(
      TestGrandchildResource::get()->isSubresourceOf(DefaultResource::get()));
  EXPECT_FALSE(
      DefaultResource::get()->isSubresourceOf(TestRootResource::get()));
  EXPECT_FALSE(isa<TestRootResource>(DefaultResource::get()));
  EXPECT_EQ(dyn_cast<TestRootResource>(DefaultResource::get()), nullptr);
}

TEST(SideEffectResourceTest, DisjointnessAndGetParent) {
  EXPECT_EQ(DefaultResource::get()->getParent(), nullptr);
  EXPECT_EQ(AutomaticAllocationScopeResource::get()->getParent(),
            DefaultResource::get());
  // TestRootResource is a subresource of DefaultResource (the root),
  // so they are not disjoint.
  EXPECT_FALSE(DefaultResource::get()->isDisjointFrom(TestRootResource::get()));
  EXPECT_TRUE(TestRootResource::get()->isSubresourceOf(DefaultResource::get()));
  EXPECT_TRUE(
      TestChildResource::get()->isSubresourceOf(TestRootResource::get()));
  EXPECT_FALSE(
      TestRootResource::get()->isSubresourceOf(TestChildResource::get()));
}
