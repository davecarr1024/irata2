// Integration tests for zero page indexed addressing mode instructions
// Phase 8.4 of ISA expansion

#include "../integration_test_helpers.h"

using namespace irata2::assembler::test;
using namespace irata2::base;
using namespace irata2::sim;

// ZPX Load/Store tests

TEST(ZeroPageIndexedTest, LdaZpxBasic) {
  // Store value to zero page, load it with X offset
  RunAsmAndCheckRegisters(R"(
    LDA #$42
    STA $15
    LDX #$05
    LDA $10,X
    HLT
  )", /*expected_a=*/Byte{0x42}, /*expected_x=*/Byte{0x05}, std::nullopt, /*max_cycles=*/200);
}

TEST(ZeroPageIndexedTest, StaZpxBasic) {
  // Store A to zero page with X offset, load it back to verify
  RunAsmAndCheckRegisters(R"(
    LDX #$03
    LDA #$99
    STA $10,X
    LDA #$00
    LDA $13
    HLT
  )", /*expected_a=*/Byte{0x99}, /*expected_x=*/Byte{0x03}, std::nullopt, /*max_cycles=*/200);
}

TEST(ZeroPageIndexedTest, LdaZpxWrapsWithinZeroPage) {
  // Test wrap-around: $FF + $05 = $04 (wraps within zero page)
  RunAsmAndCheckRegisters(R"(
    LDA #$AB
    STA $04
    LDX #$05
    LDA $FF,X
    HLT
  )", /*expected_a=*/Byte{0xAB}, /*expected_x=*/Byte{0x05}, std::nullopt, /*max_cycles=*/200);
}

TEST(ZeroPageIndexedTest, LdaZpxSetsZeroFlag) {
  RunAsmAndCheckFlags(R"(
    LDA #$00
    STA $15
    LDX #$05
    LDA #$FF
    LDA $10,X
    HLT
  )", /*expect_zero=*/true, /*expect_negative=*/false,
      /*expect_carry=*/false, /*expect_overflow=*/false, /*max_cycles=*/200);
}

TEST(ZeroPageIndexedTest, LdaZpxSetsNegativeFlag) {
  RunAsmAndCheckFlags(R"(
    LDA #$80
    STA $15
    LDX #$05
    LDA #$00
    LDA $10,X
    HLT
  )", /*expect_zero=*/false, /*expect_negative=*/true,
      /*expect_carry=*/false, /*expect_overflow=*/false, /*max_cycles=*/200);
}

// ZPX ALU operations

TEST(ZeroPageIndexedTest, AdcZpxBasic) {
  // 0x10 + 0x05 = 0x15
  RunAsmAndCheckRegisters(R"(
    LDA #$05
    STA $13
    LDX #$03
    LDA #$10
    ADC $10,X
    HLT
  )", /*expected_a=*/Byte{0x15}, /*expected_x=*/Byte{0x03}, std::nullopt, /*max_cycles=*/250);
}

TEST(ZeroPageIndexedTest, SbcZpxBasic) {
  // 0x15 - 0x05 - 1 (no carry) = 0x0F
  RunAsmAndCheckRegisters(R"(
    LDA #$05
    STA $13
    LDX #$03
    LDA #$15
    SBC $10,X
    HLT
  )", /*expected_a=*/Byte{0x0F}, /*expected_x=*/Byte{0x03}, std::nullopt, /*max_cycles=*/250);
}

TEST(ZeroPageIndexedTest, AndZpxBasic) {
  // 0xFF & 0x0F = 0x0F
  RunAsmAndCheckRegisters(R"(
    LDA #$0F
    STA $13
    LDX #$03
    LDA #$FF
    AND $10,X
    HLT
  )", /*expected_a=*/Byte{0x0F}, /*expected_x=*/Byte{0x03}, std::nullopt, /*max_cycles=*/250);
}

TEST(ZeroPageIndexedTest, OraZpxBasic) {
  // 0xF0 | 0x0F = 0xFF
  RunAsmAndCheckRegisters(R"(
    LDA #$0F
    STA $13
    LDX #$03
    LDA #$F0
    ORA $10,X
    HLT
  )", /*expected_a=*/Byte{0xFF}, /*expected_x=*/Byte{0x03}, std::nullopt, /*max_cycles=*/250);
}

TEST(ZeroPageIndexedTest, EorZpxBasic) {
  // 0xFF ^ 0xF0 = 0x0F
  RunAsmAndCheckRegisters(R"(
    LDA #$F0
    STA $13
    LDX #$03
    LDA #$FF
    EOR $10,X
    HLT
  )", /*expected_a=*/Byte{0x0F}, /*expected_x=*/Byte{0x03}, std::nullopt, /*max_cycles=*/250);
}

TEST(ZeroPageIndexedTest, CmpZpxEqual) {
  RunAsmAndCheckFlags(R"(
    LDA #$42
    STA $13
    LDX #$03
    LDA #$42
    CMP $10,X
    HLT
  )", /*expect_zero=*/true, /*expect_negative=*/false,
      /*expect_carry=*/true, /*expect_overflow=*/false, /*max_cycles=*/250);
}

// ZPX Shift operations (read-modify-write)

TEST(ZeroPageIndexedTest, AslZpxBasic) {
  // 0x01 << 1 = 0x02
  RunAsmAndCheckRegisters(R"(
    LDA #$01
    STA $13
    LDX #$03
    ASL $10,X
    LDA $13
    HLT
  )", /*expected_a=*/Byte{0x02}, /*expected_x=*/Byte{0x03}, std::nullopt, /*max_cycles=*/250);
}

TEST(ZeroPageIndexedTest, LsrZpxBasic) {
  // 0x04 >> 1 = 0x02
  RunAsmAndCheckRegisters(R"(
    LDA #$04
    STA $13
    LDX #$03
    LSR $10,X
    LDA $13
    HLT
  )", /*expected_a=*/Byte{0x02}, /*expected_x=*/Byte{0x03}, std::nullopt, /*max_cycles=*/250);
}

TEST(ZeroPageIndexedTest, RolZpxBasic) {
  // 0x01 rotated left (no carry) = 0x02
  RunAsmAndCheckRegisters(R"(
    LDA #$01
    STA $13
    LDX #$03
    ROL $10,X
    LDA $13
    HLT
  )", /*expected_a=*/Byte{0x02}, /*expected_x=*/Byte{0x03}, std::nullopt, /*max_cycles=*/250);
}

TEST(ZeroPageIndexedTest, RorZpxBasic) {
  // 0x02 rotated right (no carry) = 0x01
  RunAsmAndCheckRegisters(R"(
    LDA #$02
    STA $13
    LDX #$03
    ROR $10,X
    LDA $13
    HLT
  )", /*expected_a=*/Byte{0x01}, /*expected_x=*/Byte{0x03}, std::nullopt, /*max_cycles=*/250);
}

// ZPX Inc/Dec

TEST(ZeroPageIndexedTest, IncZpxBasic) {
  RunAsmAndCheckRegisters(R"(
    LDA #$41
    STA $13
    LDX #$03
    INC $10,X
    LDA $13
    HLT
  )", /*expected_a=*/Byte{0x42}, /*expected_x=*/Byte{0x03}, std::nullopt, /*max_cycles=*/250);
}

TEST(ZeroPageIndexedTest, DecZpxBasic) {
  RunAsmAndCheckRegisters(R"(
    LDA #$43
    STA $13
    LDX #$03
    DEC $10,X
    LDA $13
    HLT
  )", /*expected_a=*/Byte{0x42}, /*expected_x=*/Byte{0x03}, std::nullopt, /*max_cycles=*/250);
}

// LDY/STY with ZPX

TEST(ZeroPageIndexedTest, LdyZpxBasic) {
  RunAsmAndCheckRegisters(R"(
    LDA #$77
    STA $13
    LDX #$03
    LDY $10,X
    HLT
  )", std::nullopt, /*expected_x=*/Byte{0x03}, /*expected_y=*/Byte{0x77}, /*max_cycles=*/200);
}

TEST(ZeroPageIndexedTest, StyZpxBasic) {
  // Store Y to zero page with X offset, load it back to verify
  RunAsmAndCheckRegisters(R"(
    LDX #$03
    LDY #$88
    STY $10,X
    LDA $13
    HLT
  )", /*expected_a=*/Byte{0x88}, /*expected_x=*/Byte{0x03}, /*expected_y=*/Byte{0x88}, /*max_cycles=*/200);
}

// ZPY Load/Store tests

TEST(ZeroPageIndexedTest, LdxZpyBasic) {
  // Store value to zero page, load it with Y offset
  RunAsmAndCheckRegisters(R"(
    LDA #$55
    STA $15
    LDY #$05
    LDX $10,Y
    HLT
  )", std::nullopt, /*expected_x=*/Byte{0x55}, /*expected_y=*/Byte{0x05}, /*max_cycles=*/200);
}

TEST(ZeroPageIndexedTest, StxZpyBasic) {
  // Store X to zero page with Y offset, load it back to verify
  RunAsmAndCheckRegisters(R"(
    LDY #$03
    LDX #$66
    STX $10,Y
    LDA $13
    HLT
  )", /*expected_a=*/Byte{0x66}, /*expected_x=*/Byte{0x66}, /*expected_y=*/Byte{0x03}, /*max_cycles=*/200);
}

TEST(ZeroPageIndexedTest, LdxZpyWrapsWithinZeroPage) {
  // Test wrap-around: $FF + $05 = $04 (wraps within zero page)
  RunAsmAndCheckRegisters(R"(
    LDA #$CD
    STA $04
    LDY #$05
    LDX $FF,Y
    HLT
  )", std::nullopt, /*expected_x=*/Byte{0xCD}, /*expected_y=*/Byte{0x05}, /*max_cycles=*/200);
}

TEST(ZeroPageIndexedTest, LdxZpySetsZeroFlag) {
  RunAsmAndCheckFlags(R"(
    LDA #$00
    STA $15
    LDY #$05
    LDX #$FF
    LDX $10,Y
    HLT
  )", /*expect_zero=*/true, /*expect_negative=*/false,
      /*expect_carry=*/false, /*expect_overflow=*/false, /*max_cycles=*/200);
}

TEST(ZeroPageIndexedTest, LdxZpySetsNegativeFlag) {
  RunAsmAndCheckFlags(R"(
    LDA #$80
    STA $15
    LDY #$05
    LDX #$00
    LDX $10,Y
    HLT
  )", /*expect_zero=*/false, /*expect_negative=*/true,
      /*expect_carry=*/false, /*expect_overflow=*/false, /*max_cycles=*/200);
}

// Array access pattern test

TEST(ZeroPageIndexedTest, ArrayAccessPattern) {
  // Simulate array access: arr[0] + arr[1] + arr[2]
  RunAsmAndCheckRegisters(R"(
    LDA #$10
    STA $20
    LDA #$20
    STA $21
    LDA #$30
    STA $22
    LDX #$00
    LDA $20,X
    INX
    ADC $20,X
    INX
    ADC $20,X
    HLT
  )", /*expected_a=*/Byte{0x60}, /*expected_x=*/Byte{0x02}, std::nullopt, /*max_cycles=*/400);
}
