#include "irata2/sim/control.h"
#include "irata2/sim/cpu.h"
#include "irata2/hdl/cpu.h"

#include <gtest/gtest.h>

using namespace irata2::sim;

TEST(SimControlTest, AutoResetClearsOnTickClear) {
  irata2::hdl::Cpu hdl;
  Cpu sim(hdl);

  ProcessControl<true> control("auto", sim);
  control.Assert();
  EXPECT_TRUE(control.asserted());

  control.TickClear();
  EXPECT_FALSE(control.asserted());
}

TEST(SimControlTest, LatchedControlDoesNotAutoClear) {
  irata2::hdl::Cpu hdl;
  Cpu sim(hdl);

  Control<irata2::base::TickPhase::Process, false> control("latched", sim);
  control.Assert();
  EXPECT_TRUE(control.asserted());

  control.TickClear();
  EXPECT_TRUE(control.asserted());
  control.Clear();
  EXPECT_FALSE(control.asserted());
}
