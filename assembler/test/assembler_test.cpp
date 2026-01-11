#include "irata2/assembler/assembler.h"
#include "irata2/assembler/error.h"

#include <gtest/gtest.h>

using namespace irata2::assembler;

TEST(AssemblerTest, AssemblesHaltInstruction) {
  auto result = Assemble("HLT\n", "hlt.asm");
  ASSERT_EQ(result.rom.size(), 0x8000u);
  EXPECT_EQ(result.rom[0], 0x01);
  EXPECT_EQ(result.rom[1], 0xFF);
}

TEST(AssemblerTest, AssemblesCaseInsensitiveMnemonics) {
  auto result = Assemble("nOp\n", "nop.asm");
  EXPECT_EQ(result.rom[0], 0x02);
}

TEST(AssemblerTest, AssemblesByteDirective) {
  auto result = Assemble(".org $8000\n.byte $AA, %10101010, 42\n", "data.asm");
  EXPECT_EQ(result.rom[0], 0xAA);
  EXPECT_EQ(result.rom[1], 0xAA);
  EXPECT_EQ(result.rom[2], 42);
}

TEST(AssemblerTest, OrgSkipsBytesWithFill) {
  auto result = Assemble(".org $8002\nHLT\n", "org.asm");
  EXPECT_EQ(result.rom[0], 0xFF);
  EXPECT_EQ(result.rom[1], 0xFF);
  EXPECT_EQ(result.rom[2], 0x01);
}

TEST(AssemblerTest, RejectsUnknownMnemonic) {
  EXPECT_THROW(Assemble("BAD\n", "bad.asm"), AssemblerError);
}

TEST(AssemblerTest, RejectsByteOutOfRange) {
  EXPECT_THROW(Assemble(".byte 256\n", "range.asm"), AssemblerError);
}

TEST(AssemblerTest, RejectsUnknownLabel) {
  EXPECT_THROW(Assemble(".byte missing\n", "label.asm"), AssemblerError);
}

TEST(AssemblerTest, EmitsDebugJsonMetadata) {
  auto result = Assemble("start: HLT\n", "test.asm");
  EXPECT_NE(result.debug_json.find("\"version\": \"v1\""), std::string::npos);
  EXPECT_NE(result.debug_json.find("\"source_files\": [\"test.asm\"]"), std::string::npos);
  EXPECT_NE(result.debug_json.find("\"start\": \"0x8000\""), std::string::npos);
  EXPECT_NE(result.debug_json.find("\"pc_to_source\""), std::string::npos);
}
