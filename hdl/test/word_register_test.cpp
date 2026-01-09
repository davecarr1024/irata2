#include "irata2/hdl/cpu.h"
#include "irata2/hdl/word_bus.h"
#include "irata2/hdl/word_register.h"

#include <gtest/gtest.h>

using namespace irata2::hdl;

TEST(WordRegisterTest, ConstructionBindsBus) {
  Cpu cpu;
  WordBus bus("address", cpu);
  WordRegister reg("mar", cpu, bus);

  EXPECT_EQ(reg.path(), "/cpu/mar");
  EXPECT_EQ(&reg.bus(), &bus);
}
