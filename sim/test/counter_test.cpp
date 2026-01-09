#include "irata2/sim/cpu.h"
#include "irata2/hdl/cpu.h"

#include <gtest/gtest.h>

using namespace irata2::sim;

TEST(SimCounterTest, IncrementsWordCounter) {
  irata2::hdl::Cpu hdl;
  Cpu sim(hdl);

  sim.pc().increment().Assert();
  sim.Tick();

  EXPECT_EQ(sim.pc().value(), irata2::base::Word{1});
}

TEST(SimCounterTest, ResetsCounter) {
  irata2::hdl::Cpu hdl;
  Cpu sim(hdl);

  sim.pc().set_value(irata2::base::Word{0x1234});
  sim.pc().reset().Assert();
  sim.Tick();

  EXPECT_EQ(sim.pc().value(), irata2::base::Word{0});
}

TEST(SimCounterTest, IncrementsLocalCounter) {
  irata2::hdl::Cpu hdl;
  Cpu sim(hdl);

  sim.controller().sc().increment().Assert();
  sim.Tick();

  EXPECT_EQ(sim.controller().sc().value(), irata2::base::Byte{1});
}
