// Integration tests for logic immediate mode instructions (AND, ORA, EOR)
// Phase 2.2 of ISA expansion

#include "../integration_test_helpers.h"

using namespace irata2::assembler::test;
using namespace irata2::base;
using namespace irata2::sim;

// AND tests

TEST(LogicImmediateTest, AndBasic) {
  // 0xFF AND 0x0F = 0x0F
  RunAsmAndCheckRegisters(R"(
    LDA #$FF
    AND #$0F
    HLT
  )", /*expected_a=*/Byte{0x0F}, std::nullopt, std::nullopt, /*max_cycles=*/100);
}

TEST(LogicImmediateTest, AndWithZero) {
  // Any AND 0x00 = 0x00
  RunAsmAndCheckRegisters(R"(
    LDA #$FF
    AND #$00
    HLT
  )", /*expected_a=*/Byte{0x00}, std::nullopt, std::nullopt, /*max_cycles=*/100);
}

TEST(LogicImmediateTest, AndWithFF) {
  // Any AND 0xFF = same value
  RunAsmAndCheckRegisters(R"(
    LDA #$42
    AND #$FF
    HLT
  )", /*expected_a=*/Byte{0x42}, std::nullopt, std::nullopt, /*max_cycles=*/100);
}

TEST(LogicImmediateTest, AndSetsZeroFlag) {
  // Result is zero when no bits overlap
  RunAsmAndCheckFlags(R"(
    LDA #$F0
    AND #$0F
    HLT
  )", /*expect_zero=*/true, /*expect_negative=*/false,
      /*expect_carry=*/false, /*expect_overflow=*/false, /*max_cycles=*/100);
}

TEST(LogicImmediateTest, AndSetsNegativeFlag) {
  // Result has bit 7 set
  RunAsmAndCheckFlags(R"(
    LDA #$FF
    AND #$80
    HLT
  )", /*expect_zero=*/false, /*expect_negative=*/true,
      /*expect_carry=*/false, /*expect_overflow=*/false, /*max_cycles=*/100);
}

TEST(LogicImmediateTest, AndClearsFlags) {
  // Normal AND result: 0xFF AND 0x0F = 0x0F (non-zero, non-negative)
  RunAsmAndCheckFlags(R"(
    LDA #$FF
    AND #$0F
    HLT
  )", /*expect_zero=*/false, /*expect_negative=*/false,
      /*expect_carry=*/false, /*expect_overflow=*/false, /*max_cycles=*/100);
}

// ORA tests

TEST(LogicImmediateTest, OraBasic) {
  // 0xF0 OR 0x0F = 0xFF
  RunAsmAndCheckRegisters(R"(
    LDA #$F0
    ORA #$0F
    HLT
  )", /*expected_a=*/Byte{0xFF}, std::nullopt, std::nullopt, /*max_cycles=*/100);
}

TEST(LogicImmediateTest, OraWithZero) {
  // Any OR 0x00 = same value
  RunAsmAndCheckRegisters(R"(
    LDA #$42
    ORA #$00
    HLT
  )", /*expected_a=*/Byte{0x42}, std::nullopt, std::nullopt, /*max_cycles=*/100);
}

TEST(LogicImmediateTest, OraWithFF) {
  // Any OR 0xFF = 0xFF
  RunAsmAndCheckRegisters(R"(
    LDA #$00
    ORA #$FF
    HLT
  )", /*expected_a=*/Byte{0xFF}, std::nullopt, std::nullopt, /*max_cycles=*/100);
}

TEST(LogicImmediateTest, OraSetsZeroFlag) {
  // Result is zero only when both operands are zero
  RunAsmAndCheckFlags(R"(
    LDA #$00
    ORA #$00
    HLT
  )", /*expect_zero=*/true, /*expect_negative=*/false,
      /*expect_carry=*/false, /*expect_overflow=*/false, /*max_cycles=*/100);
}

TEST(LogicImmediateTest, OraSetsNegativeFlag) {
  // Result has bit 7 set
  RunAsmAndCheckFlags(R"(
    LDA #$00
    ORA #$80
    HLT
  )", /*expect_zero=*/false, /*expect_negative=*/true,
      /*expect_carry=*/false, /*expect_overflow=*/false, /*max_cycles=*/100);
}

TEST(LogicImmediateTest, OraClearsFlags) {
  // Normal OR result: non-zero, non-negative
  RunAsmAndCheckFlags(R"(
    LDA #$00
    ORA #$0F
    HLT
  )", /*expect_zero=*/false, /*expect_negative=*/false,
      /*expect_carry=*/false, /*expect_overflow=*/false, /*max_cycles=*/100);
}

// EOR tests

TEST(LogicImmediateTest, EorBasic) {
  // 0xFF XOR 0x0F = 0xF0
  RunAsmAndCheckRegisters(R"(
    LDA #$FF
    EOR #$0F
    HLT
  )", /*expected_a=*/Byte{0xF0}, std::nullopt, std::nullopt, /*max_cycles=*/100);
}

TEST(LogicImmediateTest, EorWithZero) {
  // Any XOR 0x00 = same value
  RunAsmAndCheckRegisters(R"(
    LDA #$42
    EOR #$00
    HLT
  )", /*expected_a=*/Byte{0x42}, std::nullopt, std::nullopt, /*max_cycles=*/100);
}

TEST(LogicImmediateTest, EorWithSameValue) {
  // Any XOR same = 0x00
  RunAsmAndCheckRegisters(R"(
    LDA #$42
    EOR #$42
    HLT
  )", /*expected_a=*/Byte{0x00}, std::nullopt, std::nullopt, /*max_cycles=*/100);
}

TEST(LogicImmediateTest, EorWithFF) {
  // Any XOR 0xFF = inverted
  RunAsmAndCheckRegisters(R"(
    LDA #$0F
    EOR #$FF
    HLT
  )", /*expected_a=*/Byte{0xF0}, std::nullopt, std::nullopt, /*max_cycles=*/100);
}

TEST(LogicImmediateTest, EorSetsZeroFlag) {
  // Result is zero when XOR-ing same value
  RunAsmAndCheckFlags(R"(
    LDA #$42
    EOR #$42
    HLT
  )", /*expect_zero=*/true, /*expect_negative=*/false,
      /*expect_carry=*/false, /*expect_overflow=*/false, /*max_cycles=*/100);
}

TEST(LogicImmediateTest, EorSetsNegativeFlag) {
  // Result has bit 7 set
  RunAsmAndCheckFlags(R"(
    LDA #$7F
    EOR #$FF
    HLT
  )", /*expect_zero=*/false, /*expect_negative=*/true,
      /*expect_carry=*/false, /*expect_overflow=*/false, /*max_cycles=*/100);
}

TEST(LogicImmediateTest, EorClearsFlags) {
  // Normal XOR result: non-zero, non-negative
  RunAsmAndCheckFlags(R"(
    LDA #$FF
    EOR #$0F
    HLT
  )", /*expect_zero=*/false, /*expect_negative=*/true,
      /*expect_carry=*/false, /*expect_overflow=*/false, /*max_cycles=*/100);
}

TEST(LogicImmediateTest, ChainedLogicOps) {
  // (0xFF AND 0xF0) OR 0x0A = 0xFA
  RunAsmAndCheckRegisters(R"(
    LDA #$FF
    AND #$F0
    ORA #$0A
    HLT
  )", /*expected_a=*/Byte{0xFA}, std::nullopt, std::nullopt, /*max_cycles=*/100);
}
