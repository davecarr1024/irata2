#include "irata2/hdl/byte_bus.h"
#include "irata2/hdl/cpu.h"

#include <gtest/gtest.h>

using namespace irata2::hdl;

TEST(ComponentBaseTest, RootHasNamePathAndCpu) {
  Cpu cpu;
  EXPECT_EQ(cpu.name(), "cpu");
  EXPECT_EQ(cpu.path(), "/cpu");
  EXPECT_EQ(cpu.parent_ptr(), nullptr);
  EXPECT_EQ(&cpu.cpu(), &cpu);
}

TEST(ComponentBaseTest, ChildPathUsesParent) {
  Cpu cpu;
  ByteBus bus("data", cpu);

  EXPECT_EQ(bus.name(), "data");
  EXPECT_EQ(bus.path(), "/cpu/data");
  EXPECT_EQ(bus.parent_ptr(), &cpu);
  EXPECT_EQ(&bus.cpu(), &cpu);
}
