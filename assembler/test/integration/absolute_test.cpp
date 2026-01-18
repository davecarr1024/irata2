// Integration tests for absolute addressing mode instructions

#include "../integration_test_helpers.h"

using namespace irata2::assembler::test;
using namespace irata2::base;
using namespace irata2::sim;

TEST(AbsoluteTest, StaLdaRoundTrip) {
  RunAsmAndCheckRegisters(R"(
    LDA #$42
    STA $0200
    LDA #$00
    LDA $0200
    HLT
  )", /*expected_a=*/Byte{0x42}, std::nullopt, std::nullopt, /*max_cycles=*/300);
}

TEST(AbsoluteTest, StxLdxRoundTrip) {
  RunAsmAndCheckRegisters(R"(
    LDX #$37
    STX $0201
    LDX #$00
    LDX $0201
    HLT
  )", std::nullopt, /*expected_x=*/Byte{0x37}, std::nullopt, /*max_cycles=*/300);
}

TEST(AbsoluteTest, AdcAbsBasic) {
  RunAsmAndCheckRegisters(R"(
    LDA #$05
    STA $0202
    LDA #$10
    ADC $0202
    HLT
  )", /*expected_a=*/Byte{0x15}, std::nullopt, std::nullopt, /*max_cycles=*/300);
}

TEST(AbsoluteTest, SbcAbsBasic) {
  RunAsmAndCheckRegisters(R"(
    LDA #$01
    CMP #$00
    LDA #$05
    STA $0203
    LDA #$20
    SBC $0203
    HLT
  )", /*expected_a=*/Byte{0x1B}, std::nullopt, std::nullopt, /*max_cycles=*/300);
}

TEST(AbsoluteTest, AndAbsBasic) {
  RunAsmAndCheckRegisters(R"(
    LDA #$0F
    STA $0204
    LDA #$F0
    AND $0204
    HLT
  )", /*expected_a=*/Byte{0x00}, std::nullopt, std::nullopt, /*max_cycles=*/300);
}

TEST(AbsoluteTest, OraAbsBasic) {
  RunAsmAndCheckRegisters(R"(
    LDA #$0F
    STA $0205
    LDA #$F0
    ORA $0205
    HLT
  )", /*expected_a=*/Byte{0xFF}, std::nullopt, std::nullopt, /*max_cycles=*/300);
}

TEST(AbsoluteTest, EorAbsBasic) {
  RunAsmAndCheckRegisters(R"(
    LDA #$FF
    STA $0206
    LDA #$0F
    EOR $0206
    HLT
  )", /*expected_a=*/Byte{0xF0}, std::nullopt, std::nullopt, /*max_cycles=*/300);
}

TEST(AbsoluteTest, CmpAbsEqualSetsZeroCarry) {
  RunAsmAndCheckFlags(R"(
    LDA #$22
    STA $0207
    LDA #$22
    CMP $0207
    HLT
  )", /*expect_zero=*/true, /*expect_negative=*/false,
      /*expect_carry=*/true, /*expect_overflow=*/false, /*max_cycles=*/300);
}

TEST(AbsoluteTest, AslAbsUpdatesMemory) {
  RunAsmAndCheckRegisters(R"(
    LDA #$01
    STA $0208
    ASL $0208
    LDA $0208
    HLT
  )", /*expected_a=*/Byte{0x02}, std::nullopt, std::nullopt, /*max_cycles=*/300);
}

TEST(AbsoluteTest, LsrAbsUpdatesMemory) {
  RunAsmAndCheckRegisters(R"(
    LDA #$04
    STA $0209
    LSR $0209
    LDA $0209
    HLT
  )", /*expected_a=*/Byte{0x02}, std::nullopt, std::nullopt, /*max_cycles=*/300);
}

TEST(AbsoluteTest, RolAbsUpdatesMemory) {
  RunAsmAndCheckRegisters(R"(
    LDA #$01
    STA $020A
    ROL $020A
    LDA $020A
    HLT
  )", /*expected_a=*/Byte{0x02}, std::nullopt, std::nullopt, /*max_cycles=*/300);
}

TEST(AbsoluteTest, RorAbsUpdatesMemory) {
  RunAsmAndCheckRegisters(R"(
    LDA #$02
    STA $020B
    ROR $020B
    LDA $020B
    HLT
  )", /*expected_a=*/Byte{0x01}, std::nullopt, std::nullopt, /*max_cycles=*/300);
}
