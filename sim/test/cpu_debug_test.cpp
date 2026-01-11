#include "irata2/sim.h"
#include "test_helpers.h"

#include <gtest/gtest.h>

using namespace irata2::sim;
using namespace irata2::base;

TEST(SimCpuDebugTest, InstructionAddressUsesPcWhenIpcInvalid) {
  Cpu sim = test::MakeTestCpu();
  sim.pc().set_value(Word{0x2222});

  EXPECT_EQ(sim.instruction_address().value(), 0x2222u);
}

TEST(SimCpuDebugTest, InstructionAddressUsesIpcWhenValid) {
  Cpu sim = test::MakeTestCpu();
  sim.pc().set_value(Word{0x2222});
  sim.SetIpcForTest(Word{0x1234});

  EXPECT_EQ(sim.instruction_address().value(), 0x1234u);
}

TEST(SimCpuDebugTest, InstructionSourceLocationUsesDebugSymbols) {
  Cpu sim = test::MakeTestCpu();
  DebugSymbols symbols;
  symbols.version = "v1";
  symbols.entry = Word{0x8000};
  symbols.rom_size = 0x20;
  symbols.source_root = ".";
  symbols.source_files = {"test.asm"};
  symbols.pc_to_source.emplace(0x1234,
                               SourceLocation{"test.asm", 12, 3, "lda #$01"});

  sim.LoadDebugSymbols(std::move(symbols));
  sim.SetIpcForTest(Word{0x1234});

  const auto location = sim.instruction_source_location();
  ASSERT_TRUE(location.has_value());
  EXPECT_EQ(location->file, "test.asm");
  EXPECT_EQ(location->line, 12);
  EXPECT_EQ(location->column, 3);
  EXPECT_EQ(location->text, "lda #$01");
}
