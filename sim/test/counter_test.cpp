#include "irata2/sim.h"
#include "test_helpers.h"

#include <memory>
#include <gtest/gtest.h>

using namespace irata2::sim;

TEST(SimCounterTest, IncrementsWordCounter) {
  Cpu sim = test::MakeTestCpu();

  test::AssertControl(sim.pc().increment());
  sim.Tick();

  EXPECT_EQ(sim.pc().value(), irata2::base::Word{1});
}

TEST(SimCounterTest, ResetsCounter) {
  Cpu sim = test::MakeTestCpu();

  sim.pc().set_value(irata2::base::Word{0x1234});
  test::AssertControl(sim.pc().reset());
  sim.Tick();

  EXPECT_EQ(sim.pc().value(), irata2::base::Word{0});
}

TEST(SimCounterTest, IncrementsLocalCounter) {
  Cpu sim = test::MakeTestCpu();

  test::AssertControl(sim.controller().sc().increment());
  sim.Tick();

  EXPECT_EQ(sim.controller().sc().value(), irata2::base::Byte{1});
}
