#include "irata2/hdl/cpu.h"
#include "irata2/hdl/traits.h"

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

  EXPECT_EQ(visitor.components, 1 + 2 + 4 + 9);
  EXPECT_EQ(visitor.buses, 2);
  EXPECT_EQ(visitor.registers, 4);
  EXPECT_EQ(visitor.controls, 9);
}
