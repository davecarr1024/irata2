#include "irata2/hdl/byte_bus.h"
#include "irata2/hdl.h"

#include <gtest/gtest.h>

using namespace irata2::hdl;

TEST(ComponentWithParentTest, ParentReferenceIsAvailable) {
  Cpu cpu;
  ByteBus bus("data", cpu);

  EXPECT_EQ(bus.parent().path(), "");
  EXPECT_EQ(bus.parent().name(), "cpu");
}
