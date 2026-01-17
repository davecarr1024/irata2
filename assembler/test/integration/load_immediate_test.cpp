// Integration tests for load immediate instructions (LDA, CMP)
// Tests existing instructions to verify infrastructure

#include "../integration_test_helpers.h"

using namespace irata2::assembler::test;
using namespace irata2::base;
using namespace irata2::sim;

TEST(LoadImmediateTest, LdaLoadsValue) {
  // Basic LDA - load a value into A register
  RunAsmAndCheckRegisters(R"(
    LDA #$42
    HLT
  )", /*expected_a=*/Byte{0x42}, std::nullopt, std::nullopt, /*max_cycles=*/100);
}

TEST(LoadImmediateTest, LdaLoadsZero) {
  RunAsmAndCheckRegisters(R"(
    LDA #$00
    HLT
  )", /*expected_a=*/Byte{0x00}, std::nullopt, std::nullopt, /*max_cycles=*/100);
}

TEST(LoadImmediateTest, LdaLoadsMaxValue) {
  RunAsmAndCheckRegisters(R"(
    LDA #$FF
    HLT
  )", /*expected_a=*/Byte{0xFF}, std::nullopt, std::nullopt, /*max_cycles=*/100);
}

TEST(LoadImmediateTest, LdaSetsZeroFlag) {
  // Loading 0 should set zero flag (bit 0)
  RunAsmAndCheckFlags(R"(
    LDA #$00
    HLT
  )", /*expect_zero=*/true, /*expect_negative=*/false,
      /*expect_carry=*/false, /*expect_overflow=*/false, /*max_cycles=*/100);
}

TEST(LoadImmediateTest, LdaSetsNegativeFlag) {
  // Loading 0x80 or higher sets negative flag (bit 1)
  RunAsmAndCheckFlags(R"(
    LDA #$80
    HLT
  )", /*expect_zero=*/false, /*expect_negative=*/true,
      /*expect_carry=*/false, /*expect_overflow=*/false, /*max_cycles=*/100);
}

TEST(LoadImmediateTest, LdaClearsFlags) {
  // Loading non-zero, non-negative value clears Z and N
  RunAsmAndCheckFlags(R"(
    LDA #$42
    HLT
  )", /*expect_zero=*/false, /*expect_negative=*/false,
      /*expect_carry=*/false, /*expect_overflow=*/false, /*max_cycles=*/100);
}

TEST(LoadImmediateTest, MultipleLoads) {
  // Last load wins
  RunAsmAndCheckRegisters(R"(
    LDA #$10
    LDA #$20
    LDA #$30
    HLT
  )", /*expected_a=*/Byte{0x30}, std::nullopt, std::nullopt, /*max_cycles=*/100);
}

TEST(LoadImmediateTest, CmpEqual) {
  // CMP with equal values - carry is set when A >= operand (no borrow)
  RunAsmAndCheckFlags(R"(
    LDA #$42
    CMP #$42
    HLT
  )", /*expect_zero=*/true, /*expect_negative=*/false,
      /*expect_carry=*/true, /*expect_overflow=*/false, /*max_cycles=*/100);
}

TEST(LoadImmediateTest, CmpGreater) {
  // A > operand (A=0x50, operand=0x30)
  RunAsmAndCheckFlags(R"(
    LDA #$50
    CMP #$30
    HLT
  )", /*expect_zero=*/false, /*expect_negative=*/false,
      /*expect_carry=*/true, /*expect_overflow=*/false, /*max_cycles=*/100);
}

TEST(LoadImmediateTest, CmpLess) {
  // A < operand (A=0x30, operand=0x50)
  RunAsmAndCheckFlags(R"(
    LDA #$30
    CMP #$50
    HLT
  )", /*expect_zero=*/false, /*expect_negative=*/true,
      /*expect_carry=*/false, /*expect_overflow=*/false, /*max_cycles=*/100);
}

TEST(LoadImmediateTest, ConvenienceFunction) {
  // Test convenience function
  auto a_value = RunAsmAndGetA(R"(
    LDA #$99
    HLT
  )", /*max_cycles=*/100);

  EXPECT_EQ(a_value, Byte{0x99});
}

TEST(LoadImmediateTest, CycleCount) {
  // Verify cycle counting works
  auto cycles = RunAsmAndGetCycles(R"(
    LDA #$42
    HLT
  )", /*max_cycles=*/100);

  // LDA should take ~2-3 cycles, HLT 1 cycle
  EXPECT_GE(cycles, 2);
  EXPECT_LE(cycles, 10);
}
