// Integration tests for shift accumulator mode instructions (ASL, LSR, ROL, ROR)
// Phase 2.3 of ISA expansion

#include "../integration_test_helpers.h"

using namespace irata2::assembler::test;
using namespace irata2::base;
using namespace irata2::sim;

// ASL tests (Arithmetic Shift Left)

TEST(ShiftAccumulatorTest, AslBasic) {
  // 0x01 << 1 = 0x02
  RunAsmAndCheckRegisters(R"(
    LDA #$01
    ASL
    HLT
  )", /*expected_a=*/Byte{0x02}, std::nullopt, std::nullopt, /*max_cycles=*/100);
}

TEST(ShiftAccumulatorTest, AslMultipleBits) {
  // 0x55 << 1 = 0xAA
  RunAsmAndCheckRegisters(R"(
    LDA #$55
    ASL
    HLT
  )", /*expected_a=*/Byte{0xAA}, std::nullopt, std::nullopt, /*max_cycles=*/100);
}

TEST(ShiftAccumulatorTest, AslWithCarryOut) {
  // 0x80 << 1 = 0x00, carry set
  RunAsmAndCheckFlags(R"(
    LDA #$80
    ASL
    HLT
  )", /*expect_zero=*/true, /*expect_negative=*/false,
      /*expect_carry=*/true, /*expect_overflow=*/false, /*max_cycles=*/100);
}

TEST(ShiftAccumulatorTest, AslSetsNegativeFlag) {
  // 0x40 << 1 = 0x80 (negative)
  RunAsmAndCheckFlags(R"(
    LDA #$40
    ASL
    HLT
  )", /*expect_zero=*/false, /*expect_negative=*/true,
      /*expect_carry=*/false, /*expect_overflow=*/false, /*max_cycles=*/100);
}

TEST(ShiftAccumulatorTest, AslZero) {
  // 0x00 << 1 = 0x00
  RunAsmAndCheckFlags(R"(
    LDA #$00
    ASL
    HLT
  )", /*expect_zero=*/true, /*expect_negative=*/false,
      /*expect_carry=*/false, /*expect_overflow=*/false, /*max_cycles=*/100);
}

// LSR tests (Logical Shift Right)

TEST(ShiftAccumulatorTest, LsrBasic) {
  // 0x02 >> 1 = 0x01
  RunAsmAndCheckRegisters(R"(
    LDA #$02
    LSR
    HLT
  )", /*expected_a=*/Byte{0x01}, std::nullopt, std::nullopt, /*max_cycles=*/100);
}

TEST(ShiftAccumulatorTest, LsrMultipleBits) {
  // 0xAA >> 1 = 0x55
  RunAsmAndCheckRegisters(R"(
    LDA #$AA
    LSR
    HLT
  )", /*expected_a=*/Byte{0x55}, std::nullopt, std::nullopt, /*max_cycles=*/100);
}

TEST(ShiftAccumulatorTest, LsrWithCarryOut) {
  // 0x01 >> 1 = 0x00, carry set
  RunAsmAndCheckFlags(R"(
    LDA #$01
    LSR
    HLT
  )", /*expect_zero=*/true, /*expect_negative=*/false,
      /*expect_carry=*/true, /*expect_overflow=*/false, /*max_cycles=*/100);
}

TEST(ShiftAccumulatorTest, LsrClearsNegative) {
  // LSR always clears negative (bit 7 is always 0 after shift)
  // 0x80 >> 1 = 0x40
  RunAsmAndCheckFlags(R"(
    LDA #$80
    LSR
    HLT
  )", /*expect_zero=*/false, /*expect_negative=*/false,
      /*expect_carry=*/false, /*expect_overflow=*/false, /*max_cycles=*/100);
}

TEST(ShiftAccumulatorTest, LsrZero) {
  // 0x00 >> 1 = 0x00
  RunAsmAndCheckFlags(R"(
    LDA #$00
    LSR
    HLT
  )", /*expect_zero=*/true, /*expect_negative=*/false,
      /*expect_carry=*/false, /*expect_overflow=*/false, /*max_cycles=*/100);
}

// ROL tests (Rotate Left)

TEST(ShiftAccumulatorTest, RolBasicNoCarry) {
  // 0x01 rotated left = 0x02 (carry starts at 0)
  RunAsmAndCheckRegisters(R"(
    LDA #$01
    ROL
    HLT
  )", /*expected_a=*/Byte{0x02}, std::nullopt, std::nullopt, /*max_cycles=*/100);
}

TEST(ShiftAccumulatorTest, RolWithCarryIn) {
  // Set carry first via ADC overflow, then rotate
  // 0xFF + 0x01 = 0x00 with carry set, then 0x00 ROL = 0x01
  RunAsmAndCheckRegisters(R"(
    LDA #$FF
    ADC #$01
    ROL
    HLT
  )", /*expected_a=*/Byte{0x01}, std::nullopt, std::nullopt, /*max_cycles=*/100);
}

TEST(ShiftAccumulatorTest, RolWithCarryOut) {
  // 0x80 rotated left = 0x00, carry set
  RunAsmAndCheckFlags(R"(
    LDA #$80
    ROL
    HLT
  )", /*expect_zero=*/true, /*expect_negative=*/false,
      /*expect_carry=*/true, /*expect_overflow=*/false, /*max_cycles=*/100);
}

TEST(ShiftAccumulatorTest, RolSetsNegativeFlag) {
  // 0x40 ROL = 0x80 (negative)
  RunAsmAndCheckFlags(R"(
    LDA #$40
    ROL
    HLT
  )", /*expect_zero=*/false, /*expect_negative=*/true,
      /*expect_carry=*/false, /*expect_overflow=*/false, /*max_cycles=*/100);
}

// ROR tests (Rotate Right)

TEST(ShiftAccumulatorTest, RorBasicNoCarry) {
  // 0x02 rotated right = 0x01 (carry starts at 0)
  RunAsmAndCheckRegisters(R"(
    LDA #$02
    ROR
    HLT
  )", /*expected_a=*/Byte{0x01}, std::nullopt, std::nullopt, /*max_cycles=*/100);
}

TEST(ShiftAccumulatorTest, RorWithCarryIn) {
  // Set carry first, then rotate - carry becomes bit 7
  // 0xFF + 0x01 = 0x00 with carry, then 0x00 ROR = 0x80
  RunAsmAndCheckRegisters(R"(
    LDA #$FF
    ADC #$01
    ROR
    HLT
  )", /*expected_a=*/Byte{0x80}, std::nullopt, std::nullopt, /*max_cycles=*/100);
}

TEST(ShiftAccumulatorTest, RorWithCarryOut) {
  // 0x01 rotated right = 0x00, carry set
  RunAsmAndCheckFlags(R"(
    LDA #$01
    ROR
    HLT
  )", /*expect_zero=*/true, /*expect_negative=*/false,
      /*expect_carry=*/true, /*expect_overflow=*/false, /*max_cycles=*/100);
}

TEST(ShiftAccumulatorTest, RorSetsNegativeFlag) {
  // If carry is set when rotating, bit 7 becomes 1 (negative)
  // 0xFF + 0x01 = 0x00 carry set, then 0x00 ROR = 0x80 (negative)
  RunAsmAndCheckFlags(R"(
    LDA #$FF
    ADC #$01
    ROR
    HLT
  )", /*expect_zero=*/false, /*expect_negative=*/true,
      /*expect_carry=*/false, /*expect_overflow=*/false, /*max_cycles=*/100);
}

TEST(ShiftAccumulatorTest, ChainedShifts) {
  // ASL then LSR returns to original (if no carry lost)
  // 0x40 ASL = 0x80, then LSR = 0x40
  RunAsmAndCheckRegisters(R"(
    LDA #$40
    ASL
    LSR
    HLT
  )", /*expected_a=*/Byte{0x40}, std::nullopt, std::nullopt, /*max_cycles=*/100);
}
