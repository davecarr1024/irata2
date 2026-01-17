// Integration tests for system instructions (HLT, NOP, CRS)
// These tests verify the basic integration test infrastructure works correctly

#include "../integration_test_helpers.h"

using namespace irata2::assembler::test;
using namespace irata2::base;
using namespace irata2::sim;

TEST(SystemIntegrationTest, HaltStopsCpu) {
  // Most basic test - just halt
  // Should complete in 1-2 cycles
  auto result = RunAsm("HLT\n", /*max_cycles=*/10);

  // Verify it halted normally
  EXPECT_EQ(result.reason, Cpu::HaltReason::Halt);

  // Verify it didn't take too many cycles
  EXPECT_LE(result.cycles, 5);
}

TEST(SystemIntegrationTest, NopDoesNothing) {
  // NOP should not modify any registers
  auto result = RunAsm(R"(
    NOP
    NOP
    NOP
    HLT
  )", /*max_cycles=*/20);

  ASSERT_TRUE(result.state.has_value());

  // A and X should still be 0 (default)
  EXPECT_EQ(result.state->a, Byte{0x00});
  EXPECT_EQ(result.state->x, Byte{0x00});

  // Should complete in reasonable time
  EXPECT_LE(result.cycles, 10);
}

TEST(SystemIntegrationTest, CrashStopsCpu) {
  // CRS should crash the CPU
  auto result = RunAsm("CRS\n", /*max_cycles=*/10, /*expect_halt=*/false);

  // Verify it crashed
  EXPECT_EQ(result.reason, Cpu::HaltReason::Crash);

  // Verify it didn't take too many cycles
  EXPECT_LE(result.cycles, 5);
}

TEST(SystemIntegrationTest, MultipleInstructions) {
  // Mix of different instructions
  auto result = RunAsm(R"(
    NOP
    NOP
    NOP
    HLT
  )", /*max_cycles=*/50);

  EXPECT_EQ(result.reason, Cpu::HaltReason::Halt);
  EXPECT_LE(result.cycles, 20);
}
