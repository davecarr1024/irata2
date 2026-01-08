#include "irata2/hdl/component.h"
#include "irata2/hdl/cpu.h"

#include <gtest/gtest.h>

using namespace irata2::hdl;

TEST(HdlComponentTest, CpuIsRoot) {
  Cpu cpu;
  EXPECT_EQ(&cpu.cpu(), &cpu);
}

TEST(HdlComponentTest, CpuPath) {
  Cpu cpu;
  EXPECT_EQ(cpu.path(), "/cpu");
}
