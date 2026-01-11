#include "irata2/hdl.h"
#include "irata2/hdl/traits.h"

#include <algorithm>
#include <gtest/gtest.h>

using namespace irata2::hdl;
using irata2::base::TickPhase;

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
  EXPECT_EQ(cpu.memory().path(), "memory");
  EXPECT_EQ(cpu.memory().mar().path(), "memory.mar");
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

  EXPECT_EQ(visitor.components, 69);
  EXPECT_EQ(visitor.buses, 2);
  EXPECT_EQ(visitor.registers, 9);
  EXPECT_EQ(visitor.controls, 47);
}

TEST(HdlCpuTest, GetCpuReturnsSingleton) {
  const Cpu& cpu1 = GetCpu();
  const Cpu& cpu2 = GetCpu();
  EXPECT_EQ(&cpu1, &cpu2);
}

TEST(HdlCpuTest, ControlInfoAccessible) {
  Cpu cpu;
  const auto& halt_info = cpu.halt().control_info();
  EXPECT_EQ(halt_info.phase, TickPhase::Process);
  EXPECT_TRUE(halt_info.auto_reset);
  EXPECT_EQ(halt_info.path, "halt");
}
