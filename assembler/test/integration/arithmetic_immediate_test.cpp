// Integration tests for arithmetic immediate mode instructions (ADC, SBC)
// Phase 2.1 of ISA expansion

#include "../integration_test_helpers.h"

using namespace irata2::assembler::test;
using namespace irata2::base;
using namespace irata2::sim;

// ADC tests

TEST(ArithmeticImmediateTest, AdcBasicAddition) {
  // Simple addition without carry
  RunAsmAndCheckRegisters(R"(
    LDA #$10
    ADC #$05
    HLT
  )", /*expected_a=*/Byte{0x15}, std::nullopt, std::nullopt, /*max_cycles=*/100);
}

TEST(ArithmeticImmediateTest, AdcZeroPlusZero) {
  RunAsmAndCheckRegisters(R"(
    LDA #$00
    ADC #$00
    HLT
  )", /*expected_a=*/Byte{0x00}, std::nullopt, std::nullopt, /*max_cycles=*/100);
}

TEST(ArithmeticImmediateTest, AdcMaxValue) {
  // 0xFF + 0x00 = 0xFF (no overflow)
  RunAsmAndCheckRegisters(R"(
    LDA #$FF
    ADC #$00
    HLT
  )", /*expected_a=*/Byte{0xFF}, std::nullopt, std::nullopt, /*max_cycles=*/100);
}

TEST(ArithmeticImmediateTest, AdcWithCarryOut) {
  // 0xFF + 0x01 = 0x00 with carry
  RunAsmAndCheckRegisters(R"(
    LDA #$FF
    ADC #$01
    HLT
  )", /*expected_a=*/Byte{0x00}, std::nullopt, std::nullopt, /*max_cycles=*/100);
}

TEST(ArithmeticImmediateTest, AdcSetsZeroFlag) {
  // Result is zero (0xFF + 0x01 = 0x00)
  RunAsmAndCheckFlags(R"(
    LDA #$FF
    ADC #$01
    HLT
  )", /*expect_zero=*/true, /*expect_negative=*/false,
      /*expect_carry=*/true, /*expect_overflow=*/false, /*max_cycles=*/100);
}

TEST(ArithmeticImmediateTest, AdcSetsNegativeFlag) {
  // 0x7F + 0x01 = 0x80 (bit 7 set)
  RunAsmAndCheckFlags(R"(
    LDA #$7F
    ADC #$01
    HLT
  )", /*expect_zero=*/false, /*expect_negative=*/true,
      /*expect_carry=*/false, /*expect_overflow=*/true, /*max_cycles=*/100);
}

TEST(ArithmeticImmediateTest, AdcSetsCarryFlag) {
  // 0xFF + 0x01 carries out
  RunAsmAndCheckFlags(R"(
    LDA #$FF
    ADC #$01
    HLT
  )", /*expect_zero=*/true, /*expect_negative=*/false,
      /*expect_carry=*/true, /*expect_overflow=*/false, /*max_cycles=*/100);
}

TEST(ArithmeticImmediateTest, AdcSetsOverflowFlag) {
  // Signed overflow: +127 + 1 = -128 (0x7F + 0x01 = 0x80)
  RunAsmAndCheckFlags(R"(
    LDA #$7F
    ADC #$01
    HLT
  )", /*expect_zero=*/false, /*expect_negative=*/true,
      /*expect_carry=*/false, /*expect_overflow=*/true, /*max_cycles=*/100);
}

TEST(ArithmeticImmediateTest, AdcClearsFlags) {
  // Normal addition: 0x10 + 0x05 = 0x15 (no flags)
  RunAsmAndCheckFlags(R"(
    LDA #$10
    ADC #$05
    HLT
  )", /*expect_zero=*/false, /*expect_negative=*/false,
      /*expect_carry=*/false, /*expect_overflow=*/false, /*max_cycles=*/100);
}

TEST(ArithmeticImmediateTest, AdcMultipleAdds) {
  // Chain of adds: 0x10 + 0x20 + 0x30 = 0x60
  RunAsmAndCheckRegisters(R"(
    LDA #$10
    ADC #$20
    ADC #$30
    HLT
  )", /*expected_a=*/Byte{0x60}, std::nullopt, std::nullopt, /*max_cycles=*/100);
}

// SBC tests

TEST(ArithmeticImmediateTest, SbcBasicSubtraction) {
  // Note: SBC on 6502 uses borrow = !carry
  // With carry clear (borrow = 1): 0x10 - 0x05 - 1 = 0x0A
  RunAsmAndCheckRegisters(R"(
    LDA #$10
    SBC #$05
    HLT
  )", /*expected_a=*/Byte{0x0A}, std::nullopt, std::nullopt, /*max_cycles=*/100);
}

TEST(ArithmeticImmediateTest, SbcZeroMinusZero) {
  // 0x00 - 0x00 - borrow(1) = 0xFF (wraps)
  RunAsmAndCheckRegisters(R"(
    LDA #$00
    SBC #$00
    HLT
  )", /*expected_a=*/Byte{0xFF}, std::nullopt, std::nullopt, /*max_cycles=*/100);
}

TEST(ArithmeticImmediateTest, SbcWithBorrow) {
  // 0x00 - 0x01 - borrow = 0xFE (wraps with borrow)
  RunAsmAndCheckFlags(R"(
    LDA #$00
    SBC #$01
    HLT
  )", /*expect_zero=*/false, /*expect_negative=*/true,
      /*expect_carry=*/false, /*expect_overflow=*/false, /*max_cycles=*/100);
}

TEST(ArithmeticImmediateTest, SbcSetsZeroFlag) {
  // Result is zero when A - operand - borrow = 0
  // 0x01 - 0x00 - borrow(1) = 0x00
  RunAsmAndCheckFlags(R"(
    LDA #$01
    SBC #$00
    HLT
  )", /*expect_zero=*/true, /*expect_negative=*/false,
      /*expect_carry=*/true, /*expect_overflow=*/false, /*max_cycles=*/100);
}

TEST(ArithmeticImmediateTest, SbcSetsNegativeFlag) {
  // Result has bit 7 set: 0x00 - 0x01 = 0xFF (negative)
  RunAsmAndCheckFlags(R"(
    LDA #$00
    SBC #$01
    HLT
  )", /*expect_zero=*/false, /*expect_negative=*/true,
      /*expect_carry=*/false, /*expect_overflow=*/false, /*max_cycles=*/100);
}

TEST(ArithmeticImmediateTest, SbcSetsCarryFlagNoBorrow) {
  // Carry is set when no borrow occurs: 0x10 - 0x05 = 0x0A with carry
  // Actually with initial borrow: 0x10 - 0x05 - 1 = 0x0A, carry set
  RunAsmAndCheckFlags(R"(
    LDA #$10
    SBC #$05
    HLT
  )", /*expect_zero=*/false, /*expect_negative=*/false,
      /*expect_carry=*/true, /*expect_overflow=*/false, /*max_cycles=*/100);
}

TEST(ArithmeticImmediateTest, SbcMultipleSubs) {
  // Chain of subs: 0x30 - 0x10 - borrow - 0x05 - borrow
  // First SBC: 0x30 - 0x10 - 1 = 0x1F, carry set
  // Second SBC: 0x1F - 0x05 - 0 = 0x1A, carry set (no borrow since carry was set)
  RunAsmAndCheckRegisters(R"(
    LDA #$30
    SBC #$10
    SBC #$05
    HLT
  )", /*expected_a=*/Byte{0x1A}, std::nullopt, std::nullopt, /*max_cycles=*/100);
}
