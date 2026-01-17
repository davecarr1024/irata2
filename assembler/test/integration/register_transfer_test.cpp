// Integration tests for register transfer instructions (TAX, TXA, LDX)
// Phase 3 of ISA expansion

#include "../integration_test_helpers.h"

using namespace irata2::assembler::test;
using namespace irata2::base;
using namespace irata2::sim;

// TAX tests

TEST(RegisterTransferTest, TaxBasic) {
  // Transfer A to X
  RunAsmAndCheckRegisters(R"(
    LDA #$42
    TAX
    HLT
  )", /*expected_a=*/Byte{0x42}, /*expected_x=*/Byte{0x42}, std::nullopt, /*max_cycles=*/100);
}

TEST(RegisterTransferTest, TaxZero) {
  // Transfer zero
  RunAsmAndCheckRegisters(R"(
    LDA #$00
    TAX
    HLT
  )", /*expected_a=*/Byte{0x00}, /*expected_x=*/Byte{0x00}, std::nullopt, /*max_cycles=*/100);
}

TEST(RegisterTransferTest, TaxMaxValue) {
  // Transfer max value
  RunAsmAndCheckRegisters(R"(
    LDA #$FF
    TAX
    HLT
  )", /*expected_a=*/Byte{0xFF}, /*expected_x=*/Byte{0xFF}, std::nullopt, /*max_cycles=*/100);
}

TEST(RegisterTransferTest, TaxSetsZeroFlag) {
  RunAsmAndCheckFlags(R"(
    LDA #$00
    TAX
    HLT
  )", /*expect_zero=*/true, /*expect_negative=*/false,
      /*expect_carry=*/false, /*expect_overflow=*/false, /*max_cycles=*/100);
}

TEST(RegisterTransferTest, TaxSetsNegativeFlag) {
  RunAsmAndCheckFlags(R"(
    LDA #$80
    TAX
    HLT
  )", /*expect_zero=*/false, /*expect_negative=*/true,
      /*expect_carry=*/false, /*expect_overflow=*/false, /*max_cycles=*/100);
}

// TXA tests

TEST(RegisterTransferTest, TxaBasic) {
  // Transfer X to A
  RunAsmAndCheckRegisters(R"(
    LDX #$42
    TXA
    HLT
  )", /*expected_a=*/Byte{0x42}, /*expected_x=*/Byte{0x42}, std::nullopt, /*max_cycles=*/100);
}

TEST(RegisterTransferTest, TxaZero) {
  // Transfer zero
  RunAsmAndCheckRegisters(R"(
    LDX #$00
    TXA
    HLT
  )", /*expected_a=*/Byte{0x00}, /*expected_x=*/Byte{0x00}, std::nullopt, /*max_cycles=*/100);
}

TEST(RegisterTransferTest, TxaMaxValue) {
  // Transfer max value
  RunAsmAndCheckRegisters(R"(
    LDX #$FF
    TXA
    HLT
  )", /*expected_a=*/Byte{0xFF}, /*expected_x=*/Byte{0xFF}, std::nullopt, /*max_cycles=*/100);
}

TEST(RegisterTransferTest, TxaSetsZeroFlag) {
  RunAsmAndCheckFlags(R"(
    LDX #$00
    TXA
    HLT
  )", /*expect_zero=*/true, /*expect_negative=*/false,
      /*expect_carry=*/false, /*expect_overflow=*/false, /*max_cycles=*/100);
}

TEST(RegisterTransferTest, TxaSetsNegativeFlag) {
  RunAsmAndCheckFlags(R"(
    LDX #$80
    TXA
    HLT
  )", /*expect_zero=*/false, /*expect_negative=*/true,
      /*expect_carry=*/false, /*expect_overflow=*/false, /*max_cycles=*/100);
}

// LDX tests

TEST(RegisterTransferTest, LdxBasic) {
  // Load X with immediate value
  RunAsmAndCheckRegisters(R"(
    LDX #$42
    HLT
  )", std::nullopt, /*expected_x=*/Byte{0x42}, std::nullopt, /*max_cycles=*/100);
}

TEST(RegisterTransferTest, LdxZero) {
  RunAsmAndCheckRegisters(R"(
    LDX #$00
    HLT
  )", std::nullopt, /*expected_x=*/Byte{0x00}, std::nullopt, /*max_cycles=*/100);
}

TEST(RegisterTransferTest, LdxMaxValue) {
  RunAsmAndCheckRegisters(R"(
    LDX #$FF
    HLT
  )", std::nullopt, /*expected_x=*/Byte{0xFF}, std::nullopt, /*max_cycles=*/100);
}

TEST(RegisterTransferTest, LdxSetsZeroFlag) {
  RunAsmAndCheckFlags(R"(
    LDX #$00
    HLT
  )", /*expect_zero=*/true, /*expect_negative=*/false,
      /*expect_carry=*/false, /*expect_overflow=*/false, /*max_cycles=*/100);
}

TEST(RegisterTransferTest, LdxSetsNegativeFlag) {
  RunAsmAndCheckFlags(R"(
    LDX #$80
    HLT
  )", /*expect_zero=*/false, /*expect_negative=*/true,
      /*expect_carry=*/false, /*expect_overflow=*/false, /*max_cycles=*/100);
}

// Round-trip tests

TEST(RegisterTransferTest, RoundTripAtoXtoA) {
  // A -> X -> A should preserve value
  RunAsmAndCheckRegisters(R"(
    LDA #$42
    TAX
    LDA #$00
    TXA
    HLT
  )", /*expected_a=*/Byte{0x42}, /*expected_x=*/Byte{0x42}, std::nullopt, /*max_cycles=*/100);
}

TEST(RegisterTransferTest, IndependentRegisters) {
  // A and X can hold different values
  RunAsmAndCheckRegisters(R"(
    LDA #$11
    LDX #$22
    HLT
  )", /*expected_a=*/Byte{0x11}, /*expected_x=*/Byte{0x22}, std::nullopt, /*max_cycles=*/100);
}
