#include "irata2/base/types.h"
#include "irata2/hdl/byte_bus.h"
#include "irata2/hdl/cpu.h"
#include "irata2/hdl/register.h"

#include <gtest/gtest.h>

using namespace irata2::hdl;

namespace {
class TestRegister final
    : public Register<TestRegister, irata2::base::Byte> {
 public:
  TestRegister(std::string name, ComponentBase& parent,
               const Bus<irata2::base::Byte>& bus)
      : Register<TestRegister, irata2::base::Byte>(std::move(name), parent, bus) {}
};
}  // namespace

TEST(RegisterTest, ConstructionSetsNameAndPath) {
  Cpu cpu;
  ByteBus bus("data", cpu);
  TestRegister reg("test", cpu, bus);

  EXPECT_EQ(reg.name(), "test");
  EXPECT_EQ(reg.path(), "/cpu/test");
}
