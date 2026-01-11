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
                                  {"pc.write", "controller.ipc_latch"});

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
