#include "irata2/hdl/cpu.h"
#include "irata2/hdl/word_bus.h"

#include <gtest/gtest.h>

using namespace irata2::hdl;

TEST(WordBusTest, ConstructionSetsNameAndPath) {
  Cpu cpu;
  WordBus bus("address", cpu);

  EXPECT_EQ(bus.name(), "address");
  EXPECT_EQ(bus.path(), "/cpu/address");
}
