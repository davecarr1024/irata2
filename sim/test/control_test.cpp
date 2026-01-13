#include "irata2/sim/control.h"
#include "irata2/sim.h"
#include "test_helpers.h"

#include <gtest/gtest.h>

using namespace irata2::sim;

TEST(SimControlTest, AutoResetClearsAfterTick) {
  Cpu sim;

  // Create a control and register it so it gets ticked
  ProcessControl<true> control("auto", sim);
  sim.RegisterChild(control);

  test::AssertControl(control);
  EXPECT_TRUE(test::IsAsserted(control));

  // A full tick cycle will clear auto-reset controls
  sim.Tick();
  test::SetPhase(sim, irata2::base::TickPhase::Process);
  EXPECT_FALSE(control.asserted());
}

TEST(SimControlTest, LatchedControlDoesNotAutoClear) {
  Cpu sim;

  // Create a control and register it so it gets ticked
  Control<irata2::base::TickPhase::Process, false> control("latched", sim);
  sim.RegisterChild(control);

  test::AssertControl(control);
  EXPECT_TRUE(test::IsAsserted(control));

  // A full tick cycle should NOT clear latched controls
  sim.Tick();
  test::SetPhase(sim, irata2::base::TickPhase::Process);
  EXPECT_TRUE(control.asserted());

  test::ClearControl(control);
  EXPECT_FALSE(test::IsAsserted(control));
}

TEST(SimControlTest, AssertOutsideControlPhaseThrows) {
  Cpu sim;
  ProcessControl<true> control("auto", sim);

  test::SetPhase(sim, irata2::base::TickPhase::Read);
  EXPECT_THROW(control.Assert(), SimError);
}

TEST(SimControlTest, ReadOutsideAssignedPhaseThrows) {
  Cpu sim;
  Control<irata2::base::TickPhase::Read, true> control("read", sim);

  test::AssertControl(control);
  test::SetPhase(sim, irata2::base::TickPhase::Write);
  EXPECT_THROW(control.asserted(), SimError);
}
