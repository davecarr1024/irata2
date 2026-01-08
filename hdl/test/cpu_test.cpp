#include "irata2/hdl/cpu.h"

#include <gtest/gtest.h>

using namespace irata2::hdl;

TEST(HdlCpuTest, Construction) {
  // Should be able to construct a CPU
  Cpu cpu;
  EXPECT_EQ(&cpu.cpu(), &cpu);
  EXPECT_EQ(cpu.path(), "/cpu");
}

TEST(HdlCpuTest, IsConst) {
  // HDL is immutable structural metadata
  const Cpu cpu;
  EXPECT_EQ(&cpu.cpu(), &cpu);
  EXPECT_EQ(cpu.path(), "/cpu");
}
