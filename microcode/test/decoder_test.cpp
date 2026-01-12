#include "irata2/microcode/debug/decoder.h"

#include "irata2/microcode/output/program.h"

#include <gtest/gtest.h>

using irata2::microcode::debug::MicrocodeDecoder;
using irata2::microcode::output::EncodeKey;
using irata2::microcode::output::MicrocodeKey;
using irata2::microcode::output::MicrocodeProgram;
using irata2::microcode::output::StatusBitDefinition;

namespace {

MicrocodeProgram MakeTestProgram() {
  MicrocodeProgram program;

  // Define control paths (bit 0 = halt, bit 1 = crash, bit 2 = a.read, bit 3 = x.write)
  program.control_paths = {"halt", "crash", "a.read", "x.write"};

  // Define status bits (bit 0 = zero, bit 1 = carry)
  program.status_bits = {{"zero", 0}, {"carry", 1}};

  // Add some microcode entries
  // Opcode 0, step 0, default status: halt
  program.table[EncodeKey({0, 0, 0})] = 0b0001;  // halt

  // Opcode 1, step 0, default status: a.read + x.write
  program.table[EncodeKey({1, 0, 0})] = 0b1100;  // a.read, x.write

  // Opcode 1, step 1, default status: halt
  program.table[EncodeKey({1, 1, 0})] = 0b0001;  // halt

  // Opcode 1, step 0, zero status: crash
  program.table[EncodeKey({1, 0, 0b01})] = 0b0010;  // crash (status.zero)

  // Opcode 2, step 0, carry status: a.read
  program.table[EncodeKey({2, 0, 0b10})] = 0b0100;  // a.read (status.carry)

  return program;
}

}  // namespace

TEST(MicrocodeDecoderTest, DecodesEmptyControlWord) {
  MicrocodeProgram program = MakeTestProgram();
  MicrocodeDecoder decoder(program);

  const auto controls = decoder.DecodeControlWord(0);
  EXPECT_TRUE(controls.empty());
}

TEST(MicrocodeDecoderTest, DecodesSingleControl) {
  MicrocodeProgram program = MakeTestProgram();
  MicrocodeDecoder decoder(program);

  const auto controls = decoder.DecodeControlWord(0b0001);  // halt
  ASSERT_EQ(controls.size(), 1);
  EXPECT_EQ(controls[0], "halt");
}

TEST(MicrocodeDecoderTest, DecodesMultipleControls) {
  MicrocodeProgram program = MakeTestProgram();
  MicrocodeDecoder decoder(program);

  const auto controls = decoder.DecodeControlWord(0b1100);  // a.read, x.write
  ASSERT_EQ(controls.size(), 2);
  EXPECT_EQ(controls[0], "a.read");
  EXPECT_EQ(controls[1], "x.write");
}

TEST(MicrocodeDecoderTest, DumpsEmptyProgram) {
  MicrocodeProgram program;
  program.control_paths = {"halt"};
  program.status_bits = {{"zero", 0}};

  MicrocodeDecoder decoder(program);
  const std::string dump = decoder.DumpProgram();
  EXPECT_TRUE(dump.empty());
}

TEST(MicrocodeDecoderTest, DumpsSingleInstruction) {
  MicrocodeProgram program;
  program.control_paths = {"halt"};
  program.status_bits = {{"zero", 0}};
  program.table[EncodeKey({0, 0, 0})] = 0b0001;  // opcode 0, step 0, halt

  MicrocodeDecoder decoder(program);
  const std::string dump = decoder.DumpProgram();

  EXPECT_NE(dump.find("opcode 0:"), std::string::npos);
  EXPECT_NE(dump.find("status default:"), std::string::npos);
  EXPECT_NE(dump.find("step 0: [halt]"), std::string::npos);
}

TEST(MicrocodeDecoderTest, DumpsMultipleSteps) {
  MicrocodeProgram program = MakeTestProgram();
  MicrocodeDecoder decoder(program);

  const std::string dump = decoder.DumpProgram();

  // Check for opcode headers
  EXPECT_NE(dump.find("opcode 0:"), std::string::npos);
  EXPECT_NE(dump.find("opcode 1:"), std::string::npos);
  EXPECT_NE(dump.find("opcode 2:"), std::string::npos);

  // Check for status variants
  EXPECT_NE(dump.find("status default:"), std::string::npos);
  EXPECT_NE(dump.find("status zero:"), std::string::npos);
  EXPECT_NE(dump.find("status carry:"), std::string::npos);

  // Check for specific steps
  EXPECT_NE(dump.find("step 0: [halt]"), std::string::npos);
  EXPECT_NE(dump.find("step 0: [a.read, x.write]"), std::string::npos);
  EXPECT_NE(dump.find("step 1: [halt]"), std::string::npos);
}

TEST(MicrocodeDecoderTest, DumpsInstructionWithNoMicrocode) {
  MicrocodeProgram program = MakeTestProgram();
  MicrocodeDecoder decoder(program);

  const std::string dump = decoder.DumpInstruction(99);  // Non-existent opcode

  EXPECT_NE(dump.find("opcode 99:"), std::string::npos);
  EXPECT_NE(dump.find("(no microcode)"), std::string::npos);
}

TEST(MicrocodeDecoderTest, DumpsSpecificInstruction) {
  MicrocodeProgram program = MakeTestProgram();
  MicrocodeDecoder decoder(program);

  const std::string dump = decoder.DumpInstruction(1);

  // Should include opcode 1's microcode
  EXPECT_NE(dump.find("opcode 1:"), std::string::npos);
  EXPECT_NE(dump.find("status default:"), std::string::npos);
  EXPECT_NE(dump.find("status zero:"), std::string::npos);
  EXPECT_NE(dump.find("step 0: [a.read, x.write]"), std::string::npos);
  EXPECT_NE(dump.find("step 1: [halt]"), std::string::npos);
  EXPECT_NE(dump.find("step 0: [crash]"), std::string::npos);

  // Should NOT include other opcodes
  EXPECT_EQ(dump.find("opcode 0:"), std::string::npos);
  EXPECT_EQ(dump.find("opcode 2:"), std::string::npos);
}

TEST(MicrocodeDecoderTest, DumpsInstructionSortsByStatus) {
  MicrocodeProgram program = MakeTestProgram();
  MicrocodeDecoder decoder(program);

  const std::string dump = decoder.DumpInstruction(1);

  // Default status should come before zero status
  const size_t default_pos = dump.find("status default:");
  const size_t zero_pos = dump.find("status zero:");
  ASSERT_NE(default_pos, std::string::npos);
  ASSERT_NE(zero_pos, std::string::npos);
  EXPECT_LT(default_pos, zero_pos);
}

TEST(MicrocodeDecoderTest, DecodesMultipleStatusFlags) {
  MicrocodeProgram program;
  program.control_paths = {"halt"};
  program.status_bits = {{"zero", 0}, {"carry", 1}, {"negative", 2}};
  program.table[EncodeKey({0, 0, 0b111})] = 0b0001;  // All flags set

  MicrocodeDecoder decoder(program);
  const std::string dump = decoder.DumpProgram();

  EXPECT_NE(dump.find("status zero,carry,negative:"), std::string::npos);
}
