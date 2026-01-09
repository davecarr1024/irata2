#include "irata2/microcode/ir/builder.h"

#include "irata2/hdl.h"
#include "irata2/microcode/error.h"

#include <gtest/gtest.h>

using irata2::hdl::Cpu;
using irata2::microcode::MicrocodeError;
using irata2::microcode::ir::Builder;

TEST(BuilderTest, ResolvesControlPaths) {
  Cpu cpu;
  Builder builder(cpu);

  const auto* control = builder.RequireControl("a.read", "builder-test");
  EXPECT_EQ(control, &cpu.a().read());
}

TEST(BuilderTest, ResolvesMultipleControls) {
  Cpu cpu;
  Builder builder(cpu);

  auto controls = builder.RequireControls({"a.read", "a.write"}, "multi");
  ASSERT_EQ(controls.size(), 2u);
  EXPECT_EQ(controls[0], &cpu.a().read());
  EXPECT_EQ(controls[1], &cpu.a().write());
}

TEST(BuilderTest, FailsFastOnUnknownControl) {
  Cpu cpu;
  Builder builder(cpu);

  try {
    builder.RequireControl("unknown.control", "bad");
    FAIL() << "Expected MicrocodeError";
  } catch (const MicrocodeError& error) {
    std::string message = error.what();
    EXPECT_NE(message.find("bad"), std::string::npos);
    EXPECT_NE(message.find("unknown.control"), std::string::npos);
  }
}
