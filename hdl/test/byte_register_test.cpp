#include "irata2/hdl/byte_bus.h"
#include "irata2/hdl/byte_register.h"
#include "irata2/hdl/cpu.h"

#include <gtest/gtest.h>

using namespace irata2::hdl;

TEST(ByteRegisterTest, ConstructionBindsBus) {
  Cpu cpu;
  ByteBus bus("data", cpu);
  ByteRegister reg("a", cpu, bus);

  EXPECT_EQ(reg.path(), "/cpu/a");
  EXPECT_EQ(&reg.bus(), &bus);
}
