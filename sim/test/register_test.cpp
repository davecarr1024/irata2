#include "irata2/sim.h"
#include "irata2/sim/error.h"
#include "test_helpers.h"

#include <gtest/gtest.h>

using namespace irata2::sim;

TEST(SimRegisterTest, WritesAndReadsViaBus) {
  Cpu sim = test::MakeTestCpu();

  sim.a().set_value(irata2::base::Byte{0x42});
  test::AssertControl(sim.a().write());
  test::AssertControl(sim.x().read());

  sim.Tick();

  EXPECT_EQ(sim.x().value(), irata2::base::Byte{0x42});
  EXPECT_FALSE(test::IsAsserted(sim.a().write()));
  EXPECT_FALSE(test::IsAsserted(sim.x().read()));
}

TEST(SimRegisterTest, RejectsMultipleBusWriters) {
  Cpu sim = test::MakeTestCpu();

  sim.a().set_value(irata2::base::Byte{0x10});
  sim.x().set_value(irata2::base::Byte{0x20});
  test::AssertControl(sim.a().write());
  test::AssertControl(sim.x().write());

  EXPECT_THROW(sim.Tick(), SimError);
}

TEST(SimRegisterTest, RejectsReadWithoutWriter) {
  Cpu sim = test::MakeTestCpu();

  test::AssertControl(sim.a().read());

  EXPECT_THROW(sim.Tick(), SimError);
}
