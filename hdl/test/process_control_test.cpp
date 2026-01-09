#include "irata2/base/tick_phase.h"
#include "irata2/hdl.h"
#include "irata2/hdl/process_control.h"

#include <gtest/gtest.h>

using namespace irata2::hdl;

TEST(ProcessControlTest, AutoResetTrueByDefault) {
  Cpu cpu;
  ProcessControl<> control("process", cpu);

  EXPECT_EQ(control.phase(), irata2::base::TickPhase::Process);
  EXPECT_TRUE(control.auto_reset());
}

TEST(ProcessControlTest, AutoResetFalseWhenDisabled) {
  Cpu cpu;
  ProcessControl<false> control("process", cpu);

  EXPECT_EQ(control.phase(), irata2::base::TickPhase::Process);
  EXPECT_FALSE(control.auto_reset());
}
