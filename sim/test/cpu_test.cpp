#include "irata2/sim.h"
#include "test_helpers.h"
#include "irata2/base/tick_phase.h"

#include <gtest/gtest.h>

using namespace irata2::sim;
using namespace irata2::base;

TEST(SimCpuTest, Construction) {
  Cpu sim = test::MakeTestCpu();

  EXPECT_EQ(&sim.cpu(), &sim);
  EXPECT_EQ(sim.path(), "");
}

TEST(SimCpuTest, ConstAccessors) {
  const Cpu sim = test::MakeTestCpu();

  EXPECT_EQ(&sim.cpu(), &sim);
  EXPECT_EQ(sim.path(), "");
  EXPECT_EQ(sim.current_phase(), TickPhase::None);
  EXPECT_FALSE(sim.halted());
  EXPECT_EQ(sim.cycle_count(), 0);
}

TEST(SimCpuTest, InitialState) {
  Cpu sim = test::MakeTestCpu();

  EXPECT_EQ(sim.current_phase(), TickPhase::None);
  EXPECT_FALSE(sim.halted());
  EXPECT_FALSE(sim.crashed());
  EXPECT_EQ(sim.cycle_count(), 0);
}

TEST(SimCpuTest, TickUpdatesPhases) {
  Cpu sim = test::MakeTestCpu();

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
  Cpu sim = test::MakeTestCpu();

  for (int i = 0; i < 10; ++i) {
    EXPECT_EQ(sim.cycle_count(), i);
    sim.Tick();
  }

  EXPECT_EQ(sim.cycle_count(), 10);
}

TEST(SimCpuTest, HaltedCpuDoesNotTick) {
  Cpu sim = test::MakeTestCpu();

  sim.set_halted(true);
  EXPECT_TRUE(sim.halted());

  sim.Tick();
  EXPECT_EQ(sim.cycle_count(), 0);  // Didn't increment

  sim.Tick();
  EXPECT_EQ(sim.cycle_count(), 0);  // Still didn't increment
}

TEST(SimCpuTest, HaltAndResume) {
  Cpu sim = test::MakeTestCpu();

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
  Cpu sim = test::MakeTestCpu();

  test::AssertControl(sim.halt());
  sim.Tick();
  EXPECT_TRUE(sim.halted());

  sim.Tick();
  EXPECT_EQ(sim.cycle_count(), 1);
}

TEST(SimCpuTest, CrashControlStopsCpu) {
  Cpu sim = test::MakeTestCpu();

  test::AssertControl(sim.crash());
  sim.Tick();
  EXPECT_TRUE(sim.crashed());
  EXPECT_TRUE(sim.halted());

  sim.Tick();
  EXPECT_EQ(sim.cycle_count(), 1);
}

TEST(SimCpuTest, RunUntilHaltReturnsHaltState) {
  Cpu sim = test::MakeTestCpu();

  test::AssertControl(sim.halt());
  const auto result = sim.RunUntilHalt();

  EXPECT_EQ(result.reason, Cpu::HaltReason::Halt);
  EXPECT_GT(result.cycles, 0u);
}

TEST(SimCpuTest, RunUntilHaltReturnsCrashState) {
  Cpu sim = test::MakeTestCpu();

  test::AssertControl(sim.crash());
  const auto result = sim.RunUntilHalt();

  EXPECT_EQ(result.reason, Cpu::HaltReason::Crash);
  EXPECT_GT(result.cycles, 0u);
}
