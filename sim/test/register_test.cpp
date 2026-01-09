#include "irata2/sim/cpu.h"
#include "irata2/sim/error.h"

#include <gtest/gtest.h>

using namespace irata2::sim;

namespace {
void SetSafeIr(Cpu& sim) {
  sim.controller().ir().set_value(irata2::base::Byte{0x02});
  sim.controller().sc().set_value(irata2::base::Byte{0});
}
}  // namespace

TEST(SimRegisterTest, WritesAndReadsViaBus) {
  Cpu sim;

  SetSafeIr(sim);
  sim.a().set_value(irata2::base::Byte{0x42});
  sim.a().write().Assert();
  sim.x().read().Assert();

  sim.Tick();

  EXPECT_EQ(sim.x().value(), irata2::base::Byte{0x42});
  EXPECT_FALSE(sim.a().write().asserted());
  EXPECT_FALSE(sim.x().read().asserted());
}

TEST(SimRegisterTest, RejectsMultipleBusWriters) {
  Cpu sim;

  SetSafeIr(sim);
  sim.a().set_value(irata2::base::Byte{0x10});
  sim.x().set_value(irata2::base::Byte{0x20});
  sim.a().write().Assert();
  sim.x().write().Assert();

  EXPECT_THROW(sim.Tick(), SimError);
}

TEST(SimRegisterTest, RejectsReadWithoutWriter) {
  Cpu sim;

  SetSafeIr(sim);
  sim.a().read().Assert();

  EXPECT_THROW(sim.Tick(), SimError);
}
