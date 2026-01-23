#include "irata2/sim/disassembler.h"

#include <gtest/gtest.h>

#include "irata2/assembler/assembler.h"

using irata2::assembler::Assemble;
using irata2::assembler::AssemblerResult;
using irata2::sim::Disassemble;
using irata2::sim::DisassembleOptions;

TEST(DisassemblerTest, RoundTripAssembleDisassemble) {
  const std::string program = R"(
    LDA #$01
    STA $0200
    LDX #$03
  loop:
    DEX
    BNE loop
    HLT
  )";

  AssemblerResult assembled = Assemble(program, "roundtrip.asm");
  std::vector<irata2::base::Byte> rom_bytes;
  rom_bytes.reserve(assembled.rom.size());
  for (uint8_t value : assembled.rom) {
    rom_bytes.push_back(irata2::base::Byte{value});
  }

  DisassembleOptions options;
  options.emit_org = true;
  options.emit_labels = false;
  std::string disasm = Disassemble(rom_bytes, nullptr, options);

  AssemblerResult reassembled = Assemble(disasm, "roundtrip_disasm.asm");
  EXPECT_EQ(assembled.rom, reassembled.rom);
}
