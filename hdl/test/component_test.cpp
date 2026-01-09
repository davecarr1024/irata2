#include "irata2/hdl/byte_bus.h"
#include "irata2/hdl.h"

#include <gtest/gtest.h>

using namespace irata2::hdl;

TEST(ComponentTest, VisitDispatchesToDerived) {
  Cpu cpu;
  ByteBus bus("data", cpu);

  int visits = 0;
  bus.visit([&](const auto& component) {
    (void)component;
    ++visits;
  });

  EXPECT_EQ(visits, 1);
}
