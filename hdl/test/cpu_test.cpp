#include "irata2/hdl.h"
#include "irata2/hdl/traits.h"

#include <algorithm>
#include <gtest/gtest.h>

using namespace irata2::hdl;

TEST(HdlCpuTest, Construction) {
  Cpu cpu;
  EXPECT_EQ(&cpu.cpu(), &cpu);
  EXPECT_EQ(cpu.path(), "");
}

TEST(HdlCpuTest, AccessorsReturnComponents) {
  Cpu cpu;
  EXPECT_EQ(cpu.data_bus().path(), "data_bus");
  EXPECT_EQ(cpu.address_bus().path(), "address_bus");
  EXPECT_EQ(cpu.a().path(), "a");
  EXPECT_EQ(cpu.x().path(), "x");
  EXPECT_EQ(cpu.pc().path(), "pc");
  EXPECT_EQ(cpu.mar().path(), "mar");
  EXPECT_EQ(cpu.status().path(), "status");
  EXPECT_EQ(cpu.status().zero().path(), "status.zero");
  EXPECT_EQ(cpu.status().carry().path(), "status.carry");
  EXPECT_EQ(cpu.controller().path(), "controller");
  EXPECT_EQ(cpu.halt().path(), "halt");
  EXPECT_EQ(cpu.crash().path(), "crash");
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

  EXPECT_EQ(visitor.components, 42);
  EXPECT_EQ(visitor.buses, 2);
  EXPECT_EQ(visitor.registers, 7);
  EXPECT_EQ(visitor.controls, 23);
}

TEST(HdlCpuTest, ResolveControlFindsPaths) {
  Cpu cpu;

  EXPECT_EQ(cpu.ResolveControl("a.read"), &cpu.a().read());
  EXPECT_EQ(cpu.ResolveControl("a.read"), &cpu.a().read());
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
  EXPECT_NE(std::find(paths.begin(), paths.end(), "halt"), paths.end());
  EXPECT_NE(std::find(paths.begin(), paths.end(), "controller.ir.read"), paths.end());
  EXPECT_NE(std::find(paths.begin(), paths.end(), "controller.sc.reset"), paths.end());
}
