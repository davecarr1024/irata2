#include "irata2/sim.h"
#include "irata2/sim/error.h"
#include "test_helpers.h"

#include <gtest/gtest.h>

using namespace irata2::sim;

TEST(SimRegisterTest, WritesAndReadsViaBus) {
  Cpu sim = test::MakeTestCpu();

  sim.a().set_value(irata2::base::Byte{0x42});
  sim.a().write().Assert();
  sim.x().read().Assert();

  sim.Tick();

  EXPECT_EQ(sim.x().value(), irata2::base::Byte{0x42});
  EXPECT_FALSE(sim.a().write().asserted());
  EXPECT_FALSE(sim.x().read().asserted());
}

TEST(SimRegisterTest, RejectsMultipleBusWriters) {
  Cpu sim = test::MakeTestCpu();

  sim.a().set_value(irata2::base::Byte{0x10});
  sim.x().set_value(irata2::base::Byte{0x20});
  sim.a().write().Assert();
  sim.x().write().Assert();

  EXPECT_THROW(sim.Tick(), SimError);
}

TEST(SimRegisterTest, RejectsReadWithoutWriter) {
  Cpu sim = test::MakeTestCpu();

  sim.a().read().Assert();

  EXPECT_THROW(sim.Tick(), SimError);
}
