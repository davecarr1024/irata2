#include "irata2/microcode/ir/builder.h"

#include "irata2/hdl.h"
#include "irata2/microcode/error.h"

#include <gtest/gtest.h>

using irata2::hdl::Cpu;
using irata2::microcode::MicrocodeError;
using irata2::microcode::ir::Builder;
using irata2::microcode::ir::CpuPathResolver;

TEST(BuilderTest, ResolvesControlPaths) {
  Cpu cpu;
  CpuPathResolver resolver(cpu);
  Builder builder(resolver);

  const auto* control = builder.RequireControl("a.read", "builder-test");
  EXPECT_EQ(control, &cpu.a().read().control_info());
}

TEST(BuilderTest, ResolvesMultipleControls) {
  Cpu cpu;
  CpuPathResolver resolver(cpu);
  Builder builder(resolver);

  auto controls = builder.RequireControls({"a.read", "a.write"}, "multi");
  ASSERT_EQ(controls.size(), 2u);
  EXPECT_EQ(controls[0], &cpu.a().read().control_info());
  EXPECT_EQ(controls[1], &cpu.a().write().control_info());
}

TEST(BuilderTest, FailsFastOnUnknownControl) {
  Cpu cpu;
  CpuPathResolver resolver(cpu);
  Builder builder(resolver);

  try {
    builder.RequireControl("unknown.control", "bad");
    FAIL() << "Expected MicrocodeError";
  } catch (const MicrocodeError& error) {
    std::string message = error.what();
    EXPECT_NE(message.find("bad"), std::string::npos);
    EXPECT_NE(message.find("unknown.control"), std::string::npos);
  }
}
