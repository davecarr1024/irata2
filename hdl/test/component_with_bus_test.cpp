#include "irata2/base/types.h"
#include "irata2/hdl/byte_bus.h"
#include "irata2/hdl/component_with_bus.h"
#include "irata2/hdl/cpu.h"

#include <gtest/gtest.h>

using namespace irata2::hdl;

namespace {
class TestBusComponent final
    : public ComponentWithBus<TestBusComponent, irata2::base::Byte> {
 public:
  TestBusComponent(std::string name, ComponentBase& parent,
                   const Bus<irata2::base::Byte>& bus)
      : ComponentWithBus<TestBusComponent, irata2::base::Byte>(
            std::move(name), parent, bus) {}
};
}  // namespace

TEST(ComponentWithBusTest, ExposesBusAndControls) {
  Cpu cpu;
  ByteBus bus("data", cpu);
  TestBusComponent component("device", cpu, bus);

  EXPECT_EQ(&component.bus(), &bus);
  EXPECT_EQ(component.write().phase(), irata2::base::TickPhase::Write);
  EXPECT_EQ(component.read().phase(), irata2::base::TickPhase::Read);
}

TEST(ComponentWithBusTest, VisitIncludesControls) {
  Cpu cpu;
  ByteBus bus("data", cpu);
  TestBusComponent component("device", cpu, bus);

  int visits = 0;
  component.visit([&](const auto& value) {
    (void)value;
    ++visits;
  });

  EXPECT_EQ(visits, 3);
}
