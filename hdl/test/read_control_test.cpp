#include "irata2/base/tick_phase.h"
#include "irata2/base/types.h"
#include "irata2/hdl/byte_bus.h"
#include "irata2/hdl.h"
#include "irata2/hdl/read_control.h"

#include <gtest/gtest.h>

using namespace irata2::hdl;

TEST(ReadControlTest, BindsBusAndMetadata) {
  Cpu cpu;
  ByteBus bus("data", cpu);
  ReadControl<irata2::base::Byte> control("read", cpu, bus);

  EXPECT_EQ(control.phase(), irata2::base::TickPhase::Read);
  EXPECT_TRUE(control.auto_reset());
  EXPECT_EQ(&control.bus(), &bus);
}
