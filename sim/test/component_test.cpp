#include "irata2/sim/component.h"
#include "irata2/sim.h"
#include <gtest/gtest.h>

using namespace irata2::sim;

namespace {
class DummyComponent final : public ComponentWithParent {
 public:
  DummyComponent(Component& parent, const std::string& name)
      : ComponentWithParent(parent, name) {}
};

class TickComponent final : public Component {
 public:
  Cpu& cpu() override { return *cpu_; }
  const Cpu& cpu() const override { return *cpu_; }
  irata2::base::TickPhase current_phase() const override {
    return cpu_->current_phase();
  }
  std::string path() const override { return "tick"; }
  void RegisterChild(Component& child) override { (void)child; }

  explicit TickComponent(Cpu& cpu) : cpu_(&cpu) {}

 private:
  Cpu* cpu_;
};
}  // namespace

TEST(SimComponentTest, CpuIsRoot) {
  Cpu sim;

  EXPECT_EQ(&sim.cpu(), &sim);
}

TEST(SimComponentTest, CpuPath) {
  Cpu sim;

  EXPECT_EQ(sim.path(), "");
}

TEST(SimComponentTest, ComponentWithParentAccessors) {
  Cpu sim;
  DummyComponent child(sim, "child");

  EXPECT_EQ(child.name(), "child");
  EXPECT_EQ(&child.parent(), &sim);
  EXPECT_EQ(child.path(), "child");
  EXPECT_EQ(&child.cpu(), &sim);

  const auto& const_child = child;
  EXPECT_EQ(const_child.parent().path(), "");
  EXPECT_EQ(const_child.cpu().path(), "");
}

TEST(SimComponentTest, DefaultTickMethodsAreCallable) {
  Cpu sim;
  TickComponent component(sim);

  component.TickControl();
  component.TickWrite();
  component.TickRead();
  component.TickProcess();
  component.TickClear();

  Component* base = new TickComponent(sim);
  delete base;
}
