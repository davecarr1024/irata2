#include "irata2/sim/cpu.h"

#include <gtest/gtest.h>

using irata2::base::TickPhase;

TEST(SimStatusTest, SetControlForcesBit) {
  irata2::sim::Cpu cpu;
  cpu.status().set_value(irata2::base::Byte{0});

  cpu.SetCurrentPhaseForTest(TickPhase::Control);
  cpu.status().carry().set().Assert();

  cpu.SetCurrentPhaseForTest(TickPhase::Process);
  cpu.status().carry().TickProcess();

  EXPECT_TRUE(cpu.status().carry().value());
}

TEST(SimStatusTest, ClearControlForcesBitLow) {
  irata2::sim::Cpu cpu;
  cpu.status().set_value(irata2::base::Byte{0x01});

  cpu.SetCurrentPhaseForTest(TickPhase::Control);
  cpu.status().carry().clear().Assert();

  cpu.SetCurrentPhaseForTest(TickPhase::Process);
  cpu.status().carry().TickProcess();

  EXPECT_FALSE(cpu.status().carry().value());
}

TEST(SimStatusTest, SetWinsWhenBothSetAndClearAsserted) {
  irata2::sim::Cpu cpu;
  cpu.status().set_value(irata2::base::Byte{0});

  cpu.SetCurrentPhaseForTest(TickPhase::Control);
  cpu.status().carry().set().Assert();
  cpu.status().carry().clear().Assert();

  cpu.SetCurrentPhaseForTest(TickPhase::Process);
  cpu.status().carry().TickProcess();

  EXPECT_TRUE(cpu.status().carry().value());
}
