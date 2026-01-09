#include "irata2/hdl/byte_counter.h"
#include "irata2/hdl.h"

#include <gtest/gtest.h>

using namespace irata2::hdl;

TEST(ByteCounterTest, ExposesIncrementAndResetControls) {
  Cpu cpu;
  ByteCounter counter("sc", cpu);

  EXPECT_EQ(counter.increment().phase(), irata2::base::TickPhase::Process);
  EXPECT_TRUE(counter.increment().auto_reset());
  EXPECT_EQ(counter.reset().phase(), irata2::base::TickPhase::Process);
  EXPECT_TRUE(counter.reset().auto_reset());
}

TEST(ByteCounterTest, VisitIncludesControls) {
  Cpu cpu;
  ByteCounter counter("sc", cpu);

  int visits = 0;
  counter.visit([&](const auto& component) {
    (void)component;
    ++visits;
  });

  EXPECT_EQ(visits, 3);
}
