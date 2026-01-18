// Integration tests for relative branch instructions

#include "../integration_test_helpers.h"

using namespace irata2::assembler::test;
using namespace irata2::base;

TEST(BranchTest, BeqTaken) {
  RunAsmAndCheckRegisters(R"(
    LDA #$00
    CMP #$00
    BEQ target
    LDA #$10
    HLT
  target:
    LDA #$42
    HLT
  )", /*expected_a=*/Byte{0x42}, std::nullopt, std::nullopt, /*max_cycles=*/300);
}

TEST(BranchTest, BeqNotTaken) {
  RunAsmAndCheckRegisters(R"(
    LDA #$00
    CMP #$01
    BEQ target
    LDA #$33
    HLT
  target:
    LDA #$44
    HLT
  )", /*expected_a=*/Byte{0x33}, std::nullopt, std::nullopt, /*max_cycles=*/300);
}

TEST(BranchTest, BneTakenBackward) {
  RunAsmAndCheckRegisters(R"(
    LDX #$03
  loop:
    DEX
    BNE loop
    LDA #$55
    HLT
  )", /*expected_a=*/Byte{0x55}, std::nullopt, std::nullopt, /*max_cycles=*/400);
}

TEST(BranchTest, BneNotTaken) {
  RunAsmAndCheckRegisters(R"(
    LDA #$10
    CMP #$10
    BNE target
    LDA #$66
    HLT
  target:
    LDA #$77
    HLT
  )", /*expected_a=*/Byte{0x66}, std::nullopt, std::nullopt, /*max_cycles=*/300);
}

TEST(BranchTest, BcsTaken) {
  RunAsmAndCheckRegisters(R"(
    LDA #$02
    CMP #$01
    BCS target
    LDA #$10
    HLT
  target:
    LDA #$21
    HLT
  )", /*expected_a=*/Byte{0x21}, std::nullopt, std::nullopt, /*max_cycles=*/300);
}

TEST(BranchTest, BcsNotTaken) {
  RunAsmAndCheckRegisters(R"(
    LDA #$00
    CMP #$01
    BCS target
    LDA #$22
    HLT
  target:
    LDA #$33
    HLT
  )", /*expected_a=*/Byte{0x22}, std::nullopt, std::nullopt, /*max_cycles=*/300);
}

TEST(BranchTest, BccTaken) {
  RunAsmAndCheckRegisters(R"(
    LDA #$00
    CMP #$01
    BCC target
    LDA #$10
    HLT
  target:
    LDA #$24
    HLT
  )", /*expected_a=*/Byte{0x24}, std::nullopt, std::nullopt, /*max_cycles=*/300);
}

TEST(BranchTest, BccNotTaken) {
  RunAsmAndCheckRegisters(R"(
    LDA #$02
    CMP #$01
    BCC target
    LDA #$25
    HLT
  target:
    LDA #$26
    HLT
  )", /*expected_a=*/Byte{0x25}, std::nullopt, std::nullopt, /*max_cycles=*/300);
}

TEST(BranchTest, BmiTaken) {
  RunAsmAndCheckRegisters(R"(
    LDA #$00
    CMP #$01
    BMI target
    LDA #$10
    HLT
  target:
    LDA #$27
    HLT
  )", /*expected_a=*/Byte{0x27}, std::nullopt, std::nullopt, /*max_cycles=*/300);
}

TEST(BranchTest, BmiNotTaken) {
  RunAsmAndCheckRegisters(R"(
    LDA #$01
    CMP #$01
    BMI target
    LDA #$28
    HLT
  target:
    LDA #$29
    HLT
  )", /*expected_a=*/Byte{0x28}, std::nullopt, std::nullopt, /*max_cycles=*/300);
}

TEST(BranchTest, BplTaken) {
  RunAsmAndCheckRegisters(R"(
    LDA #$02
    CMP #$01
    BPL target
    LDA #$10
    HLT
  target:
    LDA #$2A
    HLT
  )", /*expected_a=*/Byte{0x2A}, std::nullopt, std::nullopt, /*max_cycles=*/300);
}

TEST(BranchTest, BplNotTaken) {
  RunAsmAndCheckRegisters(R"(
    LDA #$00
    CMP #$01
    BPL target
    LDA #$2B
    HLT
  target:
    LDA #$2C
    HLT
  )", /*expected_a=*/Byte{0x2B}, std::nullopt, std::nullopt, /*max_cycles=*/300);
}

TEST(BranchTest, BvsTaken) {
  RunAsmAndCheckRegisters(R"(
    LDA #$00
    CMP #$01
    LDA #$50
    ADC #$50
    BVS target
    LDA #$10
    HLT
  target:
    LDA #$2D
    HLT
  )", /*expected_a=*/Byte{0x2D}, std::nullopt, std::nullopt, /*max_cycles=*/300);
}

TEST(BranchTest, BvsNotTaken) {
  RunAsmAndCheckRegisters(R"(
    LDA #$00
    CMP #$01
    LDA #$01
    ADC #$01
    BVS target
    LDA #$2E
    HLT
  target:
    LDA #$2F
    HLT
  )", /*expected_a=*/Byte{0x2E}, std::nullopt, std::nullopt, /*max_cycles=*/300);
}

TEST(BranchTest, BvcTaken) {
  RunAsmAndCheckRegisters(R"(
    LDA #$00
    CMP #$01
    LDA #$01
    ADC #$01
    BVC target
    LDA #$10
    HLT
  target:
    LDA #$30
    HLT
  )", /*expected_a=*/Byte{0x30}, std::nullopt, std::nullopt, /*max_cycles=*/300);
}

TEST(BranchTest, BvcNotTaken) {
  RunAsmAndCheckRegisters(R"(
    LDA #$00
    CMP #$01
    LDA #$50
    ADC #$50
    BVC target
    LDA #$31
    HLT
  target:
    LDA #$32
    HLT
  )", /*expected_a=*/Byte{0x31}, std::nullopt, std::nullopt, /*max_cycles=*/300);
}
