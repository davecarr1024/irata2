#include "irata2/sim/initialization.h"

#include <algorithm>
#include <gtest/gtest.h>

using namespace irata2::sim;

TEST(SimInitializationTest, DefaultHdlIsShared) {
  const auto first = DefaultHdl();
  const auto second = DefaultHdl();
  EXPECT_EQ(first.get(), second.get());
}

TEST(SimInitializationTest, DefaultMicrocodeIsShared) {
  const auto first = DefaultMicrocodeProgram();
  const auto second = DefaultMicrocodeProgram();
  EXPECT_EQ(first.get(), second.get());
}

TEST(SimInitializationTest, DefaultMicrocodeIncludesHaltControl) {
  const auto program = DefaultMicrocodeProgram();
  const auto& paths = program->control_paths;
  const bool found = std::find(paths.begin(), paths.end(), "halt") != paths.end();
  EXPECT_TRUE(found);
}
