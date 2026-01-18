// Integration tests for increment/decrement instructions

#include "../integration_test_helpers.h"

using namespace irata2::assembler::test;
using namespace irata2::base;
using namespace irata2::sim;

TEST(IncDecTest, InxDexUpdatesX) {
  RunAsmAndCheckRegisters(R"(
    LDX #$10
    INX
    INX
    DEX
    HLT
  )", std::nullopt, /*expected_x=*/Byte{0x11}, std::nullopt, /*max_cycles=*/200);
}

TEST(IncDecTest, InxSetsZeroFlag) {
  RunAsmAndCheckFlags(R"(
    LDX #$FF
    INX
    HLT
  )", /*expect_zero=*/true, /*expect_negative=*/false,
      /*expect_carry=*/false, /*expect_overflow=*/false, /*max_cycles=*/200);
}

TEST(IncDecTest, DexSetsNegativeFlag) {
  RunAsmAndCheckFlags(R"(
    LDX #$00
    DEX
    HLT
  )", /*expect_zero=*/false, /*expect_negative=*/true,
      /*expect_carry=*/false, /*expect_overflow=*/false, /*max_cycles=*/200);
}

TEST(IncDecTest, IncDecZeroPage) {
  RunAsmAndCheckRegisters(R"(
    LDA #$01
    STA $10
    INC $10
    DEC $10
    LDA $10
    HLT
  )", /*expected_a=*/Byte{0x01}, std::nullopt, std::nullopt, /*max_cycles=*/300);
}

TEST(IncDecTest, IncDecAbsolute) {
  RunAsmAndCheckRegisters(R"(
    LDA #$10
    STA $0200
    INC $0200
    INC $0200
    DEC $0200
    LDA $0200
    HLT
  )", /*expected_a=*/Byte{0x11}, std::nullopt, std::nullopt, /*max_cycles=*/300);
}
