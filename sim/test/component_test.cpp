#include "irata2/sim/component.h"
#include "irata2/sim/cpu.h"
#include "irata2/hdl/cpu.h"

#include <gtest/gtest.h>

using namespace irata2::sim;

TEST(SimComponentTest, CpuIsRoot) {
  irata2::hdl::Cpu hdl;
  Cpu sim(hdl);

  EXPECT_EQ(&sim.cpu(), &sim);
}

TEST(SimComponentTest, CpuPath) {
  irata2::hdl::Cpu hdl;
  Cpu sim(hdl);

  EXPECT_EQ(sim.path(), "/cpu");
}
