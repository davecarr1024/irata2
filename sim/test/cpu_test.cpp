#include "irata2/sim/cpu.h"
#include "irata2/hdl/cpu.h"
#include "irata2/base/tick_phase.h"

#include <gtest/gtest.h>

using namespace irata2::sim;
using namespace irata2::base;

TEST(SimCpuTest, Construction) {
  irata2::hdl::Cpu hdl;
  Cpu sim(hdl);

  EXPECT_EQ(&sim.cpu(), &sim);
  EXPECT_EQ(sim.path(), "/cpu");
}

TEST(SimCpuTest, InitialState) {
  irata2::hdl::Cpu hdl;
  Cpu sim(hdl);

  EXPECT_EQ(sim.current_phase(), TickPhase::None);
  EXPECT_FALSE(sim.halted());
  EXPECT_EQ(sim.cycle_count(), 0);
}

TEST(SimCpuTest, TickUpdatesPhases) {
  irata2::hdl::Cpu hdl;
  Cpu sim(hdl);

  // Before tick
  EXPECT_EQ(sim.current_phase(), TickPhase::None);
  EXPECT_EQ(sim.cycle_count(), 0);

  // Execute one tick
  sim.Tick();

  // After tick
  EXPECT_EQ(sim.current_phase(), TickPhase::None);
  EXPECT_EQ(sim.cycle_count(), 1);
}

TEST(SimCpuTest, MultipleTicks) {
  irata2::hdl::Cpu hdl;
  Cpu sim(hdl);

  for (int i = 0; i < 10; ++i) {
    EXPECT_EQ(sim.cycle_count(), i);
    sim.Tick();
  }

  EXPECT_EQ(sim.cycle_count(), 10);
}

TEST(SimCpuTest, HaltedCpuDoesNotTick) {
  irata2::hdl::Cpu hdl;
  Cpu sim(hdl);

  sim.set_halted(true);
  EXPECT_TRUE(sim.halted());

  sim.Tick();
  EXPECT_EQ(sim.cycle_count(), 0);  // Didn't increment

  sim.Tick();
  EXPECT_EQ(sim.cycle_count(), 0);  // Still didn't increment
}

TEST(SimCpuTest, HaltAndResume) {
  irata2::hdl::Cpu hdl;
  Cpu sim(hdl);

  // Run a few cycles
  sim.Tick();
  sim.Tick();
  EXPECT_EQ(sim.cycle_count(), 2);

  // Halt
  sim.set_halted(true);
  sim.Tick();
  EXPECT_EQ(sim.cycle_count(), 2);  // Didn't increment

  // Resume
  sim.set_halted(false);
  sim.Tick();
  EXPECT_EQ(sim.cycle_count(), 3);  // Incremented again
}
