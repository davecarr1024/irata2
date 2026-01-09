#include "irata2/base/tick_phase.h"
#include "irata2/base/types.h"
#include "irata2/hdl/byte_bus.h"
#include "irata2/hdl.h"
#include "irata2/hdl/write_control.h"

#include <gtest/gtest.h>

using namespace irata2::hdl;

TEST(WriteControlTest, BindsBusAndMetadata) {
  Cpu cpu;
  ByteBus bus("data", cpu);
  WriteControl<irata2::base::Byte> control("write", cpu, bus);

  EXPECT_EQ(control.phase(), irata2::base::TickPhase::Write);
  EXPECT_TRUE(control.auto_reset());
  EXPECT_EQ(&control.bus(), &bus);
}
