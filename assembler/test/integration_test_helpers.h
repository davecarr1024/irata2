#ifndef IRATA2_ASSEMBLER_TEST_INTEGRATION_TEST_HELPERS_H
#define IRATA2_ASSEMBLER_TEST_INTEGRATION_TEST_HELPERS_H

#include "irata2/assembler/assembler.h"
#include "irata2/sim/cpu.h"
#include "irata2/sim/initialization.h"
#include "irata2/base/types.h"
#include <gtest/gtest.h>
#include <optional>
#include <string>
#include <vector>

namespace irata2::assembler::test {

/**
 * @brief Run assembled code with safety timeout.
 *
 * CRITICAL: Always use max_cycles to prevent infinite loops!
 *
 * @param asm_code Assembly source code
 * @param max_cycles Maximum cycles before timeout (default: 1000)
 * @param expect_halt If true, expects normal halt; if false, expects crash
 * @return Final CPU run result
 */
inline sim::Cpu::RunResult RunAsm(
    const std::string& asm_code,
    uint64_t max_cycles = 1000,
    bool expect_halt = true) {

  // Assemble the code
  auto result = Assemble(asm_code, "test.asm");

  // Create CPU with assembled ROM
  sim::Cpu cpu(
      sim::DefaultHdl(),
      sim::DefaultMicrocodeProgram(),
      std::vector<base::Byte>(result.rom.begin(), result.rom.end()));

  // Set entry point to ROM start (0x8000)
  cpu.pc().set_value(base::Word{0x8000});

  // Run with timeout and capture state
  auto run_result = cpu.RunUntilHalt(max_cycles, /*capture_state=*/true);

  // Verify didn't timeout
  if (run_result.reason == sim::Cpu::HaltReason::Timeout) {
    ADD_FAILURE() << "Test timed out after " << max_cycles
                  << " cycles. Possible infinite loop!\n"
                  << "Cycles executed: " << run_result.cycles << "\n"
                  << "ASM code:\n" << asm_code;
  }

  // Verify halt vs crash
  if (expect_halt) {
    EXPECT_EQ(run_result.reason, sim::Cpu::HaltReason::Halt)
        << "Expected normal halt, got crash";
  } else {
    EXPECT_EQ(run_result.reason, sim::Cpu::HaltReason::Crash)
        << "Expected crash, got normal halt";
  }

  return run_result;
}

/**
 * @brief Run assembled code and check final register values.
 *
 * @param asm_code Assembly source code
 * @param expected_a Expected value of A register (nullopt to skip check)
 * @param expected_x Expected value of X register (nullopt to skip check)
 * @param expected_status Expected value of status register (nullopt to skip check)
 * @param max_cycles Maximum cycles before timeout
 */
inline void RunAsmAndCheckRegisters(
    const std::string& asm_code,
    std::optional<base::Byte> expected_a = std::nullopt,
    std::optional<base::Byte> expected_x = std::nullopt,
    std::optional<base::Byte> expected_status = std::nullopt,
    uint64_t max_cycles = 1000) {

  auto result = RunAsm(asm_code, max_cycles);

  ASSERT_TRUE(result.state.has_value()) << "No CPU state captured";

  if (expected_a.has_value()) {
    EXPECT_EQ(result.state->a, *expected_a)
        << "A register mismatch: expected " << expected_a->to_string()
        << ", got " << result.state->a.to_string();
  }

  if (expected_x.has_value()) {
    EXPECT_EQ(result.state->x, *expected_x)
        << "X register mismatch: expected " << expected_x->to_string()
        << ", got " << result.state->x.to_string();
  }

  if (expected_status.has_value()) {
    EXPECT_EQ(result.state->status, *expected_status)
        << "Status register mismatch: expected " << expected_status->to_string()
        << ", got " << result.state->status.to_string();
  }
}

/**
 * @brief Run assembled code and check status flags.
 *
 * Status flag bits (6502 ordering):
 * - Bit 0: Carry (C)
 * - Bit 1: Zero (Z)
 * - Bit 2: Interrupt Disable (I)
 * - Bit 3: Decimal (D)
 * - Bit 4: Break (B)
 * - Bit 5: Unused
 * - Bit 6: Overflow (V)
 * - Bit 7: Negative (N)
 *
 * @param asm_code Assembly source code
 * @param expect_zero Expected state of Zero flag
 * @param expect_negative Expected state of Negative flag
 * @param expect_carry Expected state of Carry flag
 * @param expect_overflow Expected state of Overflow flag
 * @param max_cycles Maximum cycles before timeout
 */
inline void RunAsmAndCheckFlags(
    const std::string& asm_code,
    bool expect_zero = false,
    bool expect_negative = false,
    bool expect_carry = false,
    bool expect_overflow = false,
    uint64_t max_cycles = 1000) {

  auto result = RunAsm(asm_code, max_cycles);
  ASSERT_TRUE(result.state.has_value()) << "No CPU state captured";

  base::Byte status = result.state->status;
  uint8_t status_val = status.value();

  // 6502 bit ordering
  bool actual_carry = (status_val & 0x01) != 0;     // bit 0
  bool actual_zero = (status_val & 0x02) != 0;      // bit 1
  bool actual_overflow = (status_val & 0x40) != 0;  // bit 6
  bool actual_negative = (status_val & 0x80) != 0;  // bit 7

  EXPECT_EQ(actual_zero, expect_zero)
      << "Zero flag mismatch (bit 0): expected " << expect_zero
      << ", got " << actual_zero
      << " (status = " << status.to_string() << ")";

  EXPECT_EQ(actual_negative, expect_negative)
      << "Negative flag mismatch (bit 1): expected " << expect_negative
      << ", got " << actual_negative
      << " (status = " << status.to_string() << ")";

  EXPECT_EQ(actual_carry, expect_carry)
      << "Carry flag mismatch (bit 2): expected " << expect_carry
      << ", got " << actual_carry
      << " (status = " << status.to_string() << ")";

  EXPECT_EQ(actual_overflow, expect_overflow)
      << "Overflow flag mismatch (bit 3): expected " << expect_overflow
      << ", got " << actual_overflow
      << " (status = " << status.to_string() << ")";
}

/**
 * @brief Run assembled code and return final A register value.
 *
 * Convenience function for simple register checks.
 *
 * @param asm_code Assembly source code
 * @param max_cycles Maximum cycles before timeout
 * @return Final A register value
 */
inline base::Byte RunAsmAndGetA(
    const std::string& asm_code,
    uint64_t max_cycles = 1000) {

  auto result = RunAsm(asm_code, max_cycles);
  EXPECT_TRUE(result.state.has_value()) << "No CPU state captured";
  return result.state ? result.state->a : base::Byte{0};
}

/**
 * @brief Run assembled code and return final X register value.
 *
 * @param asm_code Assembly source code
 * @param max_cycles Maximum cycles before timeout
 * @return Final X register value
 */
inline base::Byte RunAsmAndGetX(
    const std::string& asm_code,
    uint64_t max_cycles = 1000) {

  auto result = RunAsm(asm_code, max_cycles);
  EXPECT_TRUE(result.state.has_value()) << "No CPU state captured";
  return result.state ? result.state->x : base::Byte{0};
}

/**
 * @brief Run assembled code and return cycle count.
 *
 * Useful for verifying instruction timing.
 *
 * @param asm_code Assembly source code
 * @param max_cycles Maximum cycles before timeout
 * @return Number of cycles executed
 */
inline uint64_t RunAsmAndGetCycles(
    const std::string& asm_code,
    uint64_t max_cycles = 1000) {

  auto result = RunAsm(asm_code, max_cycles);
  return result.cycles;
}

}  // namespace irata2::assembler::test

#endif  // IRATA2_ASSEMBLER_TEST_INTEGRATION_TEST_HELPERS_H
