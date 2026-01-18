// Integration tests for zero page addressing mode instructions
// Phase 4 of ISA expansion

#include "../integration_test_helpers.h"

using namespace irata2::assembler::test;
using namespace irata2::base;
using namespace irata2::sim;

// Load/Store tests - test by storing a value and loading it back

TEST(ZeroPageTest, StaLdaRoundTrip) {
  // Store value to zero page, load it back
  RunAsmAndCheckRegisters(R"(
    LDA #$42
    STA $10
    LDA #$00
    LDA $10
    HLT
  )", /*expected_a=*/Byte{0x42}, std::nullopt, std::nullopt, /*max_cycles=*/200);
}

TEST(ZeroPageTest, StaLdaZero) {
  RunAsmAndCheckRegisters(R"(
    LDA #$00
    STA $20
    LDA #$FF
    LDA $20
    HLT
  )", /*expected_a=*/Byte{0x00}, std::nullopt, std::nullopt, /*max_cycles=*/200);
}

TEST(ZeroPageTest, StaLdaMaxValue) {
  RunAsmAndCheckRegisters(R"(
    LDA #$FF
    STA $30
    LDA #$00
    LDA $30
    HLT
  )", /*expected_a=*/Byte{0xFF}, std::nullopt, std::nullopt, /*max_cycles=*/200);
}

TEST(ZeroPageTest, LdaZpSetsZeroFlag) {
  RunAsmAndCheckFlags(R"(
    LDA #$00
    STA $10
    LDA #$FF
    LDA $10
    HLT
  )", /*expect_zero=*/true, /*expect_negative=*/false,
      /*expect_carry=*/false, /*expect_overflow=*/false, /*max_cycles=*/200);
}

TEST(ZeroPageTest, LdaZpSetsNegativeFlag) {
  RunAsmAndCheckFlags(R"(
    LDA #$80
    STA $10
    LDA #$00
    LDA $10
    HLT
  )", /*expect_zero=*/false, /*expect_negative=*/true,
      /*expect_carry=*/false, /*expect_overflow=*/false, /*max_cycles=*/200);
}

// LDX/STX zero page tests

TEST(ZeroPageTest, StxLdxRoundTrip) {
  RunAsmAndCheckRegisters(R"(
    LDX #$42
    STX $10
    LDX #$00
    LDX $10
    HLT
  )", std::nullopt, /*expected_x=*/Byte{0x42}, std::nullopt, /*max_cycles=*/200);
}

TEST(ZeroPageTest, StxLdxZero) {
  RunAsmAndCheckRegisters(R"(
    LDX #$00
    STX $20
    LDX #$FF
    LDX $20
    HLT
  )", std::nullopt, /*expected_x=*/Byte{0x00}, std::nullopt, /*max_cycles=*/200);
}

TEST(ZeroPageTest, LdxZpSetsZeroFlag) {
  RunAsmAndCheckFlags(R"(
    LDX #$00
    STX $10
    LDX #$FF
    LDX $10
    HLT
  )", /*expect_zero=*/true, /*expect_negative=*/false,
      /*expect_carry=*/false, /*expect_overflow=*/false, /*max_cycles=*/200);
}

TEST(ZeroPageTest, LdxZpSetsNegativeFlag) {
  RunAsmAndCheckFlags(R"(
    LDX #$80
    STX $10
    LDX #$00
    LDX $10
    HLT
  )", /*expect_zero=*/false, /*expect_negative=*/true,
      /*expect_carry=*/false, /*expect_overflow=*/false, /*max_cycles=*/200);
}

// A and X can access different ZP locations

TEST(ZeroPageTest, IndependentAandXZeroPage) {
  RunAsmAndCheckRegisters(R"(
    LDA #$11
    STA $10
    LDX #$22
    STX $20
    LDA #$00
    LDX #$00
    LDA $10
    LDX $20
    HLT
  )", /*expected_a=*/Byte{0x11}, /*expected_x=*/Byte{0x22}, std::nullopt, /*max_cycles=*/300);
}

// Zero Page ALU Operations - ADC

TEST(ZeroPageTest, AdcZpBasic) {
  // 0x10 + 0x05 = 0x15
  RunAsmAndCheckRegisters(R"(
    LDA #$05
    STA $10
    LDA #$10
    ADC $10
    HLT
  )", /*expected_a=*/Byte{0x15}, std::nullopt, std::nullopt, /*max_cycles=*/200);
}

TEST(ZeroPageTest, AdcZpWithCarry) {
  // 0xFF + 0x01 = 0x00 with carry
  RunAsmAndCheckFlags(R"(
    LDA #$01
    STA $10
    LDA #$FF
    ADC $10
    HLT
  )", /*expect_zero=*/true, /*expect_negative=*/false,
      /*expect_carry=*/true, /*expect_overflow=*/false, /*max_cycles=*/200);
}

// Zero Page ALU Operations - SBC

TEST(ZeroPageTest, SbcZpBasic) {
  // 0x15 - 0x05 - 1 (no carry) = 0x0F (since carry not set, subtract extra 1)
  // SBC without carry set subtracts an extra 1
  RunAsmAndCheckRegisters(R"(
    LDA #$05
    STA $10
    LDA #$15
    SBC $10
    HLT
  )", /*expected_a=*/Byte{0x0F}, std::nullopt, std::nullopt, /*max_cycles=*/200);
}

// Zero Page Logic Operations - AND

TEST(ZeroPageTest, AndZpBasic) {
  // 0xFF & 0x0F = 0x0F
  RunAsmAndCheckRegisters(R"(
    LDA #$0F
    STA $10
    LDA #$FF
    AND $10
    HLT
  )", /*expected_a=*/Byte{0x0F}, std::nullopt, std::nullopt, /*max_cycles=*/200);
}

TEST(ZeroPageTest, AndZpSetsZeroFlag) {
  // 0xF0 & 0x0F = 0x00
  RunAsmAndCheckFlags(R"(
    LDA #$0F
    STA $10
    LDA #$F0
    AND $10
    HLT
  )", /*expect_zero=*/true, /*expect_negative=*/false,
      /*expect_carry=*/false, /*expect_overflow=*/false, /*max_cycles=*/200);
}

// Zero Page Logic Operations - ORA

TEST(ZeroPageTest, OraZpBasic) {
  // 0xF0 | 0x0F = 0xFF
  RunAsmAndCheckRegisters(R"(
    LDA #$0F
    STA $10
    LDA #$F0
    ORA $10
    HLT
  )", /*expected_a=*/Byte{0xFF}, std::nullopt, std::nullopt, /*max_cycles=*/200);
}

// Zero Page Logic Operations - EOR

TEST(ZeroPageTest, EorZpBasic) {
  // 0xFF ^ 0xF0 = 0x0F
  RunAsmAndCheckRegisters(R"(
    LDA #$F0
    STA $10
    LDA #$FF
    EOR $10
    HLT
  )", /*expected_a=*/Byte{0x0F}, std::nullopt, std::nullopt, /*max_cycles=*/200);
}

TEST(ZeroPageTest, EorZpSetsZeroFlag) {
  // 0xAA ^ 0xAA = 0x00
  RunAsmAndCheckFlags(R"(
    LDA #$AA
    STA $10
    LDA #$AA
    EOR $10
    HLT
  )", /*expect_zero=*/true, /*expect_negative=*/false,
      /*expect_carry=*/false, /*expect_overflow=*/false, /*max_cycles=*/200);
}

// Zero Page Compare - CMP

TEST(ZeroPageTest, CmpZpEqual) {
  RunAsmAndCheckFlags(R"(
    LDA #$42
    STA $10
    LDA #$42
    CMP $10
    HLT
  )", /*expect_zero=*/true, /*expect_negative=*/false,
      /*expect_carry=*/true, /*expect_overflow=*/false, /*max_cycles=*/200);
}

TEST(ZeroPageTest, CmpZpGreater) {
  // A > operand -> carry set, zero clear
  RunAsmAndCheckFlags(R"(
    LDA #$30
    STA $10
    LDA #$50
    CMP $10
    HLT
  )", /*expect_zero=*/false, /*expect_negative=*/false,
      /*expect_carry=*/true, /*expect_overflow=*/false, /*max_cycles=*/200);
}

TEST(ZeroPageTest, CmpZpLess) {
  // A < operand -> carry clear
  RunAsmAndCheckFlags(R"(
    LDA #$50
    STA $10
    LDA #$30
    CMP $10
    HLT
  )", /*expect_zero=*/false, /*expect_negative=*/true,
      /*expect_carry=*/false, /*expect_overflow=*/false, /*max_cycles=*/200);
}

// Zero Page Shift Operations - ASL (read-modify-write)

TEST(ZeroPageTest, AslZpBasic) {
  // 0x01 << 1 = 0x02, then load result to A for checking
  RunAsmAndCheckRegisters(R"(
    LDA #$01
    STA $10
    ASL $10
    LDA $10
    HLT
  )", /*expected_a=*/Byte{0x02}, std::nullopt, std::nullopt, /*max_cycles=*/200);
}

TEST(ZeroPageTest, AslZpSetsCarry) {
  // 0x80 << 1 = 0x00 with carry
  RunAsmAndCheckFlags(R"(
    LDA #$80
    STA $10
    ASL $10
    HLT
  )", /*expect_zero=*/true, /*expect_negative=*/false,
      /*expect_carry=*/true, /*expect_overflow=*/false, /*max_cycles=*/200);
}

// Zero Page Shift Operations - LSR (read-modify-write)

TEST(ZeroPageTest, LsrZpBasic) {
  // 0x04 >> 1 = 0x02
  RunAsmAndCheckRegisters(R"(
    LDA #$04
    STA $10
    LSR $10
    LDA $10
    HLT
  )", /*expected_a=*/Byte{0x02}, std::nullopt, std::nullopt, /*max_cycles=*/200);
}

TEST(ZeroPageTest, LsrZpSetsCarry) {
  // 0x01 >> 1 = 0x00 with carry
  RunAsmAndCheckFlags(R"(
    LDA #$01
    STA $10
    LSR $10
    HLT
  )", /*expect_zero=*/true, /*expect_negative=*/false,
      /*expect_carry=*/true, /*expect_overflow=*/false, /*max_cycles=*/200);
}

// Zero Page Shift Operations - ROL (read-modify-write)

TEST(ZeroPageTest, RolZpBasic) {
  // 0x01 rotated left (no carry) = 0x02
  RunAsmAndCheckRegisters(R"(
    LDA #$01
    STA $10
    ROL $10
    LDA $10
    HLT
  )", /*expected_a=*/Byte{0x02}, std::nullopt, std::nullopt, /*max_cycles=*/200);
}

TEST(ZeroPageTest, RolZpWithCarryIn) {
  // With carry set, ROL puts 1 in bit 0
  // First ADC to set carry, then ROL
  RunAsmAndCheckRegisters(R"(
    LDA #$FF
    ADC #$01
    LDA #$00
    STA $10
    ROL $10
    LDA $10
    HLT
  )", /*expected_a=*/Byte{0x01}, std::nullopt, std::nullopt, /*max_cycles=*/300);
}

// Zero Page Shift Operations - ROR (read-modify-write)

TEST(ZeroPageTest, RorZpBasic) {
  // 0x02 rotated right (no carry) = 0x01
  RunAsmAndCheckRegisters(R"(
    LDA #$02
    STA $10
    ROR $10
    LDA $10
    HLT
  )", /*expected_a=*/Byte{0x01}, std::nullopt, std::nullopt, /*max_cycles=*/200);
}

TEST(ZeroPageTest, RorZpWithCarryIn) {
  // With carry set, ROR puts 1 in bit 7
  RunAsmAndCheckRegisters(R"(
    LDA #$FF
    ADC #$01
    LDA #$00
    STA $10
    ROR $10
    LDA $10
    HLT
  )", /*expected_a=*/Byte{0x80}, std::nullopt, std::nullopt, /*max_cycles=*/300);
}

// Multiple ZP addresses test

TEST(ZeroPageTest, MultipleZeroPageAddresses) {
  // Store and load different values at different ZP addresses
  RunAsmAndCheckRegisters(R"(
    LDA #$11
    STA $10
    LDA #$22
    STA $20
    LDA #$33
    STA $30
    LDA $10
    ADC $20
    ADC $30
    HLT
  )", /*expected_a=*/Byte{0x66}, std::nullopt, std::nullopt, /*max_cycles=*/400);
}
