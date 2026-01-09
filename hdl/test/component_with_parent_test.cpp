#include "irata2/hdl/byte_bus.h"
#include "irata2/hdl/cpu.h"

#include <gtest/gtest.h>

using namespace irata2::hdl;

TEST(ComponentWithParentTest, ParentReferenceIsAvailable) {
  Cpu cpu;
  ByteBus bus("data", cpu);

  EXPECT_EQ(bus.parent().path(), "/cpu");
  EXPECT_EQ(bus.parent().name(), "cpu");
}
