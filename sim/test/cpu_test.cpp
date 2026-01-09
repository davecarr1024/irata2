#include "irata2/sim.h"
#include "irata2/base/tick_phase.h"

#include <gtest/gtest.h>

using namespace irata2::sim;
using namespace irata2::base;

namespace {
void SetSafeIr(Cpu& sim) {
  sim.controller().ir().set_value(irata2::base::Byte{0x02});
  sim.controller().sc().set_value(irata2::base::Byte{0});
}
}  // namespace

TEST(SimCpuTest, Construction) {
  Cpu sim;

  EXPECT_EQ(&sim.cpu(), &sim);
  EXPECT_EQ(sim.path(), "");
}

TEST(SimCpuTest, ConstAccessors) {
  const Cpu sim;

  EXPECT_EQ(&sim.cpu(), &sim);
  EXPECT_EQ(sim.path(), "");
  EXPECT_EQ(sim.current_phase(), TickPhase::None);
  EXPECT_FALSE(sim.halted());
  EXPECT_EQ(sim.cycle_count(), 0);
}

TEST(SimCpuTest, InitialState) {
  Cpu sim;

  EXPECT_EQ(sim.current_phase(), TickPhase::None);
  EXPECT_FALSE(sim.halted());
  EXPECT_FALSE(sim.crashed());
  EXPECT_EQ(sim.cycle_count(), 0);
}

TEST(SimCpuTest, TickUpdatesPhases) {
  Cpu sim;

  SetSafeIr(sim);
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
  Cpu sim;

  SetSafeIr(sim);
  for (int i = 0; i < 10; ++i) {
    EXPECT_EQ(sim.cycle_count(), i);
    sim.Tick();
  }

  EXPECT_EQ(sim.cycle_count(), 10);
}

TEST(SimCpuTest, HaltedCpuDoesNotTick) {
  Cpu sim;

  sim.set_halted(true);
  EXPECT_TRUE(sim.halted());

  sim.Tick();
  EXPECT_EQ(sim.cycle_count(), 0);  // Didn't increment

  sim.Tick();
  EXPECT_EQ(sim.cycle_count(), 0);  // Still didn't increment
}

TEST(SimCpuTest, HaltAndResume) {
  Cpu sim;

  SetSafeIr(sim);
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

TEST(SimCpuTest, HaltControlStopsCpu) {
  Cpu sim;

  SetSafeIr(sim);
  sim.halt().Assert();
  sim.Tick();
  EXPECT_TRUE(sim.halted());

  sim.Tick();
  EXPECT_EQ(sim.cycle_count(), 1);
}

TEST(SimCpuTest, CrashControlStopsCpu) {
  Cpu sim;

  SetSafeIr(sim);
  sim.crash().Assert();
  sim.Tick();
  EXPECT_TRUE(sim.crashed());
  EXPECT_TRUE(sim.halted());

  sim.Tick();
  EXPECT_EQ(sim.cycle_count(), 1);
}
