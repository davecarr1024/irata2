#include "irata2/base/tick_phase.h"
#include "irata2/hdl/control.h"
#include "irata2/hdl/cpu.h"

#include <gtest/gtest.h>

using namespace irata2::hdl;

namespace {
class TestControl final
    : public Control<TestControl, int, irata2::base::TickPhase::Process, false> {
 public:
  TestControl(std::string name, ComponentBase& parent)
      : Control<TestControl, int, irata2::base::TickPhase::Process, false>(
            std::move(name), parent) {}
};
}  // namespace

TEST(ControlTest, PhaseAndAutoResetAreExposed) {
  Cpu cpu;
  TestControl control("test", cpu);

  EXPECT_EQ(control.phase(), irata2::base::TickPhase::Process);
  EXPECT_FALSE(control.auto_reset());
}
