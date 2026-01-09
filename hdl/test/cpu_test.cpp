#include "irata2/hdl/cpu.h"
#include "irata2/hdl/traits.h"

#include <algorithm>
#include <gtest/gtest.h>

using namespace irata2::hdl;

TEST(HdlCpuTest, Construction) {
  Cpu cpu;
  EXPECT_EQ(&cpu.cpu(), &cpu);
  EXPECT_EQ(cpu.path(), "/cpu");
}

TEST(HdlCpuTest, AccessorsReturnComponents) {
  Cpu cpu;
  EXPECT_EQ(cpu.data_bus().path(), "/cpu/data_bus");
  EXPECT_EQ(cpu.address_bus().path(), "/cpu/address_bus");
  EXPECT_EQ(cpu.a().path(), "/cpu/a");
  EXPECT_EQ(cpu.x().path(), "/cpu/x");
  EXPECT_EQ(cpu.pc().path(), "/cpu/pc");
  EXPECT_EQ(cpu.mar().path(), "/cpu/mar");
  EXPECT_EQ(cpu.controller().path(), "/cpu/controller");
  EXPECT_EQ(cpu.halt().path(), "/cpu/halt");
  EXPECT_EQ(cpu.crash().path(), "/cpu/crash");
}

namespace {
struct CountVisitor {
  int components = 0;
  int buses = 0;
  int registers = 0;
  int controls = 0;

  template <typename T>
  void operator()(const T&) {
    ++components;
    if constexpr (is_bus_v<T>) {
      ++buses;
    }
    if constexpr (is_register_v<T>) {
      ++registers;
    }
    if constexpr (is_control_v<T>) {
      ++controls;
    }
  }
};
}  // namespace

TEST(HdlCpuTest, VisitCountsComponents) {
  Cpu cpu;
  CountVisitor visitor;

  cpu.visit(visitor);

  EXPECT_EQ(visitor.components, 30);
  EXPECT_EQ(visitor.buses, 2);
  EXPECT_EQ(visitor.registers, 6);
  EXPECT_EQ(visitor.controls, 20);
}

TEST(HdlCpuTest, ResolveControlFindsPaths) {
  Cpu cpu;

  EXPECT_EQ(cpu.ResolveControl("a.read"), &cpu.a().read());
  EXPECT_EQ(cpu.ResolveControl("/cpu/a/read"), &cpu.a().read());
  EXPECT_EQ(cpu.ResolveControl("controller.ir.write"), &cpu.controller().ir().write());
  EXPECT_EQ(cpu.ResolveControl("controller.sc.increment"),
            &cpu.controller().sc().increment());
  EXPECT_EQ(cpu.ResolveControl("halt"), &cpu.halt());
}

TEST(HdlCpuTest, ResolveControlRejectsUnknownPath) {
  Cpu cpu;
  EXPECT_THROW(cpu.ResolveControl("nope.control"), PathResolutionError);
}

TEST(HdlCpuTest, ResolveControlRejectsEmptyPath) {
  Cpu cpu;
  EXPECT_THROW(cpu.ResolveControl(""), PathResolutionError);
}

TEST(HdlCpuTest, AllControlPathsIsSortedAndComplete) {
  Cpu cpu;
  auto paths = cpu.AllControlPaths();

  EXPECT_TRUE(std::is_sorted(paths.begin(), paths.end()));
  EXPECT_NE(std::find(paths.begin(), paths.end(), "/cpu/halt"), paths.end());
  EXPECT_NE(std::find(paths.begin(), paths.end(), "/cpu/controller/ir/read"), paths.end());
  EXPECT_NE(std::find(paths.begin(), paths.end(), "/cpu/controller/sc/reset"), paths.end());
}
