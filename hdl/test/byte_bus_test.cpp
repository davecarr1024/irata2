#include "irata2/hdl/byte_bus.h"
#include "irata2/hdl/cpu.h"

#include <gtest/gtest.h>

using namespace irata2::hdl;

TEST(ByteBusTest, ConstructionSetsNameAndPath) {
  Cpu cpu;
  ByteBus bus("data", cpu);

  EXPECT_EQ(bus.name(), "data");
  EXPECT_EQ(bus.path(), "/cpu/data");
}
