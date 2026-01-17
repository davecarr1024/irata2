#include "irata2/sim/debug_dump.h"

#include "irata2/microcode/encoder/control_encoder.h"
#include "irata2/microcode/output/program.h"
#include "irata2/sim.h"

#include <algorithm>
#include <stdexcept>
#include <gtest/gtest.h>

using namespace irata2::sim;
using namespace irata2::base;

namespace {
std::shared_ptr<irata2::microcode::output::MicrocodeProgram> MakeTraceProgram(
    const std::vector<std::string>& control_paths,
    std::initializer_list<std::string_view> controls) {
  auto program =
      std::make_shared<irata2::microcode::output::MicrocodeProgram>();
  program->control_paths = control_paths;

  uint64_t word = 0;
  for (const auto& control : controls) {
    auto it = std::find(control_paths.begin(), control_paths.end(), control);
    if (it == control_paths.end()) {
      throw std::runtime_error("missing control path");
    }
    const size_t index = static_cast<size_t>(std::distance(control_paths.begin(), it));
    word |= (1ULL << index);
  }

  irata2::microcode::output::MicrocodeKey key{0x02, 0, 0};
  program->table.emplace(irata2::microcode::output::EncodeKey(key), word);
  return program;
}
}  // namespace

TEST(DebugDumpTest, IncludesTraceAndSourceLocation) {
  irata2::microcode::encoder::ControlEncoder encoder(*DefaultHdl());
  auto program = MakeTraceProgram(encoder.control_paths(),
                                  {"pc.write", "controller.ipc.latch"});

  Cpu cpu(DefaultHdl(), program);
  cpu.pc().set_value(Word{0x4000});
  cpu.a().set_value(Byte{0x12});
  cpu.x().set_value(Byte{0x34});
  cpu.status().set_value(Byte{0x80});

  DebugSymbols symbols;
  symbols.version = "v1";
  symbols.entry = Word{0x4000};
  symbols.rom_size = 0x20;
  symbols.source_root = ".";
  symbols.source_files = {"test.asm"};
  symbols.pc_to_source.emplace(0x4000,
                               SourceLocation{"test.asm", 12, 3, "lda #$01"});
  cpu.LoadDebugSymbols(std::move(symbols));
  cpu.EnableTrace(4);

  cpu.Tick();

  const std::string dump = FormatDebugDump(cpu, "crash");
  EXPECT_NE(dump.find("crash"), std::string::npos);
  EXPECT_NE(dump.find("0x4000"), std::string::npos);
  EXPECT_NE(dump.find("test.asm:12:3"), std::string::npos);
  EXPECT_NE(dump.find("trace (1 entries)"), std::string::npos);
}

TEST(DebugDumpTest, IncludesAllExpectedFields) {
  irata2::microcode::encoder::ControlEncoder encoder(*DefaultHdl());
  auto program = MakeTraceProgram(encoder.control_paths(),
                                  {"pc.write", "controller.ipc.latch"});

  Cpu cpu(DefaultHdl(), program);
  cpu.pc().set_value(Word{0x8000});
  cpu.a().set_value(Byte{0xAB});
  cpu.x().set_value(Byte{0xCD});
  cpu.status().set_value(Byte{0x82});  // N=1, Z=1
  cpu.controller().ir().set_value(Byte{0x02});
  cpu.controller().sc().set_value(Byte{0x00});

  DebugSymbols symbols;
  symbols.version = "v1";
  symbols.entry = Word{0x8000};
  symbols.rom_size = 0x8000;
  symbols.source_root = "/src";
  symbols.source_files = {"main.asm"};
  symbols.pc_to_source.emplace(0x8000,
                               SourceLocation{"main.asm", 5, 1, "LDA #$AB"});
  cpu.LoadDebugSymbols(std::move(symbols));
  cpu.EnableTrace(8);

  cpu.Tick();

  const std::string dump = FormatDebugDump(cpu, "timeout");

  // Verify reason appears
  EXPECT_NE(dump.find("Debug dump (timeout)"), std::string::npos);

  // Verify cycle count appears
  EXPECT_NE(dump.find("cycle:"), std::string::npos);

  // Verify instruction address and source location
  EXPECT_NE(dump.find("instruction:"), std::string::npos);
  EXPECT_NE(dump.find("main.asm:5:1"), std::string::npos);
  EXPECT_NE(dump.find("LDA #$AB"), std::string::npos);

  // Verify PC, IPC, IR, SC
  EXPECT_NE(dump.find("pc:"), std::string::npos);
  EXPECT_NE(dump.find("ipc:"), std::string::npos);
  EXPECT_NE(dump.find("ir:"), std::string::npos);
  EXPECT_NE(dump.find("sc:"), std::string::npos);

  // Verify registers
  EXPECT_NE(dump.find("a: 0x"), std::string::npos);
  EXPECT_NE(dump.find("x: 0x"), std::string::npos);
  EXPECT_NE(dump.find("sr: 0x"), std::string::npos);

  // Verify flags
  EXPECT_NE(dump.find("flags:"), std::string::npos);
  EXPECT_NE(dump.find("N="), std::string::npos);
  EXPECT_NE(dump.find("Z="), std::string::npos);
  EXPECT_NE(dump.find("C="), std::string::npos);

  // Verify buses
  EXPECT_NE(dump.find("buses:"), std::string::npos);
  EXPECT_NE(dump.find("data="), std::string::npos);
  EXPECT_NE(dump.find("address="), std::string::npos);

  // Verify trace section
  EXPECT_NE(dump.find("trace ("), std::string::npos);
  EXPECT_NE(dump.find("entries)"), std::string::npos);
}

TEST(DebugDumpTest, HandlesUnknownSourceLocation) {
  irata2::microcode::encoder::ControlEncoder encoder(*DefaultHdl());
  auto program = MakeTraceProgram(encoder.control_paths(),
                                  {"pc.write", "controller.ipc.latch"});

  Cpu cpu(DefaultHdl(), program);
  cpu.pc().set_value(Word{0x9000});
  cpu.EnableTrace(4);

  // No debug symbols loaded - should show "unknown" for source location
  cpu.Tick();

  const std::string dump = FormatDebugDump(cpu, "halt");
  EXPECT_NE(dump.find("halt"), std::string::npos);
  EXPECT_NE(dump.find("unknown"), std::string::npos);
}
