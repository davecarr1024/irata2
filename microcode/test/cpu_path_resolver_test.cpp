#include "irata2/microcode/ir/cpu_path_resolver.h"

#include "irata2/hdl.h"
#include "irata2/microcode/error.h"

#include <algorithm>
#include <gtest/gtest.h>

using irata2::hdl::Cpu;
using irata2::microcode::MicrocodeError;
using irata2::microcode::ir::CpuPathResolver;

TEST(CpuPathResolverTest, ResolvesKnownControl) {
  Cpu cpu;
  CpuPathResolver resolver(cpu);

  const auto* control = resolver.RequireControl("halt", "resolver-test");
  EXPECT_EQ(control, &cpu.halt().control_info());
}

TEST(CpuPathResolverTest, ReportsUnknownControlPath) {
  Cpu cpu;
  CpuPathResolver resolver(cpu);

  try {
    resolver.RequireControl("unknown.control", "bad");
    FAIL() << "Expected MicrocodeError";
  } catch (const MicrocodeError& error) {
    const std::string message = error.what();
    EXPECT_NE(message.find("unknown.control"), std::string::npos);
    EXPECT_NE(message.find("bad"), std::string::npos);
  }
}

TEST(CpuPathResolverTest, RejectsEmptyPath) {
  Cpu cpu;
  CpuPathResolver resolver(cpu);

  EXPECT_THROW(resolver.RequireControl("", "empty"), MicrocodeError);
}

TEST(CpuPathResolverTest, AllControlPathsAreSorted) {
  Cpu cpu;
  CpuPathResolver resolver(cpu);

  const auto& paths = resolver.AllControlPaths();
  EXPECT_FALSE(paths.empty());
  EXPECT_TRUE(std::is_sorted(paths.begin(), paths.end()));
}
