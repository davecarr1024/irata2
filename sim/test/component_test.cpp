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

// Note: Tick methods are now protected and tested via full CPU integration tests
// This aligns with the design principle of "Full CPU testing only"
