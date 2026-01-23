#include "irata2/hdl/latched_control.h"
#include "irata2/hdl.h"

#include <gtest/gtest.h>

using namespace irata2::hdl;

TEST(LatchedControlTest, ExposesPhaseAndAutoReset) {
  Cpu cpu;
  LatchedControl<irata2::base::TickPhase::Process> control("latched", cpu);

  EXPECT_EQ(control.phase(), irata2::base::TickPhase::Process);
  EXPECT_FALSE(control.auto_reset());
}
