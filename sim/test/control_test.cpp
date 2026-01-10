#include "irata2/sim/control.h"
#include "irata2/sim.h"
#include "test_helpers.h"

#include <gtest/gtest.h>

using namespace irata2::sim;

TEST(SimControlTest, AutoResetClearsOnTickClear) {
  Cpu sim;

  ProcessControl<true> control("auto", sim);
  test::AssertControl(control);
  EXPECT_TRUE(test::IsAsserted(control));

  control.TickClear();
  EXPECT_FALSE(test::IsAsserted(control));
}

TEST(SimControlTest, LatchedControlDoesNotAutoClear) {
  Cpu sim;

  Control<irata2::base::TickPhase::Process, false> control("latched", sim);
  test::AssertControl(control);
  EXPECT_TRUE(test::IsAsserted(control));

  control.TickClear();
  EXPECT_TRUE(test::IsAsserted(control));
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
