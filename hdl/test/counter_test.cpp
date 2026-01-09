#include "irata2/base/tick_phase.h"
#include "irata2/base/types.h"
#include "irata2/hdl/counter.h"
#include "irata2/hdl.h"
#include "irata2/hdl/word_bus.h"

#include <gtest/gtest.h>

using namespace irata2::hdl;

TEST(CounterTest, ExposesControls) {
  Cpu cpu;
  WordBus bus("address", cpu);
  Counter<irata2::base::Word> counter("pc", cpu, bus);

  EXPECT_EQ(counter.write().phase(), irata2::base::TickPhase::Write);
  EXPECT_EQ(counter.read().phase(), irata2::base::TickPhase::Read);
  EXPECT_EQ(counter.increment().phase(), irata2::base::TickPhase::Process);
  EXPECT_TRUE(counter.increment().auto_reset());
  EXPECT_EQ(counter.reset().phase(), irata2::base::TickPhase::Process);
  EXPECT_TRUE(counter.reset().auto_reset());
}

TEST(CounterTest, VisitIncludesControls) {
  Cpu cpu;
  WordBus bus("address", cpu);
  Counter<irata2::base::Word> counter("pc", cpu, bus);

  int visits = 0;
  counter.visit([&](const auto& component) {
    (void)component;
    ++visits;
  });

  EXPECT_EQ(visits, 5);
}
