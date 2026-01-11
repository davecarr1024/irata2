#include "irata2/sim.h"

#include <gtest/gtest.h>

using namespace irata2;

TEST(BusTest, ReadBeforeWriteThrows) {
  sim::Cpu cpu;

  cpu.SetCurrentPhaseForTest(base::TickPhase::Read);
  EXPECT_THROW(cpu.data_bus().Read("reader"), sim::SimError);
}

TEST(BusTest, WriteOutsideWritePhaseThrows) {
  sim::Cpu cpu;

  cpu.SetCurrentPhaseForTest(base::TickPhase::Read);
  EXPECT_THROW(cpu.data_bus().Write(base::Byte{0x12}, "writer"), sim::SimError);
}

TEST(BusTest, ReadAfterWriteInSameTickSucceeds) {
  sim::Cpu cpu;

  cpu.SetCurrentPhaseForTest(base::TickPhase::Write);
  cpu.data_bus().Write(base::Byte{0x5A}, "writer");

  cpu.SetCurrentPhaseForTest(base::TickPhase::Read);
  EXPECT_EQ(cpu.data_bus().Read("reader").value(), 0x5A);
}
