#include "irata2/sim.h"
#include "test_helpers.h"

#include <gtest/gtest.h>

using namespace irata2::sim;
using irata2::base::Byte;

namespace {

// Helper to set ALU opcode bits
void SetAluOpcode(Cpu& cpu, uint8_t opcode) {
  test::SetPhase(cpu, irata2::base::TickPhase::Control);
  if (opcode & 0x01) {
    cpu.alu().opcode_bit_0().Assert();
  }
  if (opcode & 0x02) {
    cpu.alu().opcode_bit_1().Assert();
  }
  if (opcode & 0x04) {
    cpu.alu().opcode_bit_2().Assert();
  }
  if (opcode & 0x08) {
    cpu.alu().opcode_bit_3().Assert();
  }
}

}  // namespace

// ============================================================================
// ADD Tests (Opcode 0x1)
// ============================================================================

TEST(AluTest, AddWithoutCarry) {
  Cpu cpu = test::MakeTestCpu();

  // Setup: 0x10 + 0x20 = 0x30
  cpu.alu().lhs().set_value(Byte{0x10});
  cpu.alu().rhs().set_value(Byte{0x20});
  cpu.status().carry().Set(false);  // Carry clear
  SetAluOpcode(cpu, 0x1);  // ADD

  test::SetPhase(cpu, irata2::base::TickPhase::Process);
  cpu.alu().TickProcess();

  EXPECT_EQ(cpu.alu().result().value(), Byte{0x30});
  EXPECT_FALSE(cpu.status().carry().value());  // No carry out
  EXPECT_FALSE(cpu.status().overflow().value());  // No overflow
}

TEST(AluTest, AddWithCarryIn) {
  Cpu cpu = test::MakeTestCpu();

  // Setup: 0x10 + 0x20 + 1 (carry) = 0x31
  cpu.alu().lhs().set_value(Byte{0x10});
  cpu.alu().rhs().set_value(Byte{0x20});
  cpu.status().carry().Set(true);  // Carry set
  SetAluOpcode(cpu, 0x1);  // ADD

  test::SetPhase(cpu, irata2::base::TickPhase::Process);
  cpu.alu().TickProcess();

  EXPECT_EQ(cpu.alu().result().value(), Byte{0x31});
  EXPECT_FALSE(cpu.status().carry().value());  // No carry out
  EXPECT_FALSE(cpu.status().overflow().value());  // No overflow
}

TEST(AluTest, AddWithCarryOut) {
  Cpu cpu = test::MakeTestCpu();

  // Setup: 0xFF + 0x02 = 0x101 (carry out)
  cpu.alu().lhs().set_value(Byte{0xFF});
  cpu.alu().rhs().set_value(Byte{0x02});
  cpu.status().carry().Set(false);
  SetAluOpcode(cpu, 0x1);  // ADD

  test::SetPhase(cpu, irata2::base::TickPhase::Process);
  cpu.alu().TickProcess();

  EXPECT_EQ(cpu.alu().result().value(), Byte{0x01});  // Wraps to 0x01
  EXPECT_TRUE(cpu.status().carry().value());  // Carry out set
}

TEST(AluTest, AddWithCarryInAndOut) {
  Cpu cpu = test::MakeTestCpu();

  // Setup: 0xFF + 0x01 + 1 (carry) = 0x101 (carry out)
  cpu.alu().lhs().set_value(Byte{0xFF});
  cpu.alu().rhs().set_value(Byte{0x01});
  cpu.status().carry().Set(true);
  SetAluOpcode(cpu, 0x1);  // ADD

  test::SetPhase(cpu, irata2::base::TickPhase::Process);
  cpu.alu().TickProcess();

  EXPECT_EQ(cpu.alu().result().value(), Byte{0x01});
  EXPECT_TRUE(cpu.status().carry().value());  // Carry out set
}

TEST(AluTest, AddPositiveOverflow) {
  Cpu cpu = test::MakeTestCpu();

  // Setup: 0x7F (127) + 0x01 (1) = 0x80 (-128 in signed)
  // This is overflow: positive + positive = negative
  cpu.alu().lhs().set_value(Byte{0x7F});
  cpu.alu().rhs().set_value(Byte{0x01});
  cpu.status().carry().Set(false);
  SetAluOpcode(cpu, 0x1);  // ADD

  test::SetPhase(cpu, irata2::base::TickPhase::Process);
  cpu.alu().TickProcess();

  EXPECT_EQ(cpu.alu().result().value(), Byte{0x80});
  EXPECT_FALSE(cpu.status().carry().value());  // No carry
  EXPECT_TRUE(cpu.status().overflow().value());  // Overflow set
}

TEST(AluTest, AddNegativeOverflow) {
  Cpu cpu = test::MakeTestCpu();

  // Setup: 0x80 (-128) + 0x80 (-128) = 0x100 (0 with carry, overflow)
  // This is overflow: negative + negative = positive (with carry)
  cpu.alu().lhs().set_value(Byte{0x80});
  cpu.alu().rhs().set_value(Byte{0x80});
  cpu.status().carry().Set(false);
  SetAluOpcode(cpu, 0x1);  // ADD

  test::SetPhase(cpu, irata2::base::TickPhase::Process);
  cpu.alu().TickProcess();

  EXPECT_EQ(cpu.alu().result().value(), Byte{0x00});
  EXPECT_TRUE(cpu.status().carry().value());  // Carry set
  EXPECT_TRUE(cpu.status().overflow().value());  // Overflow set
}

TEST(AluTest, AddNoOverflowDifferentSigns) {
  Cpu cpu = test::MakeTestCpu();

  // Setup: 0x7F (127) + 0x81 (-127) = 0x00
  // No overflow: positive + negative can't overflow
  cpu.alu().lhs().set_value(Byte{0x7F});
  cpu.alu().rhs().set_value(Byte{0x81});
  cpu.status().carry().Set(false);
  SetAluOpcode(cpu, 0x1);  // ADD

  test::SetPhase(cpu, irata2::base::TickPhase::Process);
  cpu.alu().TickProcess();

  EXPECT_EQ(cpu.alu().result().value(), Byte{0x00});
  EXPECT_TRUE(cpu.status().carry().value());  // Carry set
  EXPECT_FALSE(cpu.status().overflow().value());  // No overflow
}

TEST(AluTest, AddZeroToZero) {
  Cpu cpu = test::MakeTestCpu();

  cpu.alu().lhs().set_value(Byte{0x00});
  cpu.alu().rhs().set_value(Byte{0x00});
  cpu.status().carry().Set(false);
  SetAluOpcode(cpu, 0x1);  // ADD

  test::SetPhase(cpu, irata2::base::TickPhase::Process);
  cpu.alu().TickProcess();

  EXPECT_EQ(cpu.alu().result().value(), Byte{0x00});
  EXPECT_FALSE(cpu.status().carry().value());
  EXPECT_FALSE(cpu.status().overflow().value());
}

// ============================================================================
// INC Tests (Opcode 0x3)
// ============================================================================

TEST(AluTest, IncBasic) {
  Cpu cpu = test::MakeTestCpu();

  // Setup: 0x10 + 1 = 0x11
  cpu.alu().lhs().set_value(Byte{0x10});
  SetAluOpcode(cpu, 0x3);  // INC

  test::SetPhase(cpu, irata2::base::TickPhase::Process);
  cpu.alu().TickProcess();

  EXPECT_EQ(cpu.alu().result().value(), Byte{0x11});
}

TEST(AluTest, IncIgnoresCarry) {
  Cpu cpu = test::MakeTestCpu();

  // Setup: 0x10 + 1 = 0x11 (carry flag should be ignored)
  cpu.alu().lhs().set_value(Byte{0x10});
  cpu.status().carry().Set(true);  // Carry set, but INC ignores it
  SetAluOpcode(cpu, 0x3);  // INC

  test::SetPhase(cpu, irata2::base::TickPhase::Process);
  cpu.alu().TickProcess();

  EXPECT_EQ(cpu.alu().result().value(), Byte{0x11});
  EXPECT_TRUE(cpu.status().carry().value());  // Carry unchanged
}

TEST(AluTest, IncWrapAround) {
  Cpu cpu = test::MakeTestCpu();

  // Setup: 0xFF + 1 = 0x00 (wraps around)
  cpu.alu().lhs().set_value(Byte{0xFF});
  cpu.status().carry().Set(false);
  SetAluOpcode(cpu, 0x3);  // INC

  test::SetPhase(cpu, irata2::base::TickPhase::Process);
  cpu.alu().TickProcess();

  EXPECT_EQ(cpu.alu().result().value(), Byte{0x00});
  EXPECT_FALSE(cpu.status().carry().value());  // INC doesn't set carry
}

TEST(AluTest, IncDoesNotAffectCarry) {
  Cpu cpu = test::MakeTestCpu();

  // Verify INC doesn't modify carry flag
  cpu.alu().lhs().set_value(Byte{0x7F});
  cpu.status().carry().Set(true);
  SetAluOpcode(cpu, 0x3);  // INC

  test::SetPhase(cpu, irata2::base::TickPhase::Process);
  cpu.alu().TickProcess();

  EXPECT_EQ(cpu.alu().result().value(), Byte{0x80});
  EXPECT_TRUE(cpu.status().carry().value());  // Carry still set
}

TEST(AluTest, IncDoesNotAffectOverflow) {
  Cpu cpu = test::MakeTestCpu();

  // Verify INC doesn't modify overflow flag
  cpu.alu().lhs().set_value(Byte{0x7F});
  cpu.status().overflow().Set(false);
  SetAluOpcode(cpu, 0x3);  // INC

  test::SetPhase(cpu, irata2::base::TickPhase::Process);
  cpu.alu().TickProcess();

  EXPECT_EQ(cpu.alu().result().value(), Byte{0x80});
  EXPECT_FALSE(cpu.status().overflow().value());  // Overflow still clear
}

TEST(AluTest, IncZero) {
  Cpu cpu = test::MakeTestCpu();

  cpu.alu().lhs().set_value(Byte{0x00});
  SetAluOpcode(cpu, 0x3);  // INC

  test::SetPhase(cpu, irata2::base::TickPhase::Process);
  cpu.alu().TickProcess();

  EXPECT_EQ(cpu.alu().result().value(), Byte{0x01});
}

// ============================================================================
// SUB Tests (Opcode 0x2) - Verify existing implementation still works
// ============================================================================

TEST(AluTest, SubBasic) {
  Cpu cpu = test::MakeTestCpu();

  // Setup: 0x30 - 0x10 = 0x20 (with carry set = no borrow)
  cpu.alu().lhs().set_value(Byte{0x30});
  cpu.alu().rhs().set_value(Byte{0x10});
  cpu.status().carry().Set(true);  // Carry set = no borrow
  SetAluOpcode(cpu, 0x2);  // SUB

  test::SetPhase(cpu, irata2::base::TickPhase::Process);
  cpu.alu().TickProcess();

  EXPECT_EQ(cpu.alu().result().value(), Byte{0x20});
  EXPECT_TRUE(cpu.status().carry().value());  // No borrow, carry still set
}

TEST(AluTest, SubWithBorrow) {
  Cpu cpu = test::MakeTestCpu();

  // Setup: 0x30 - 0x10 - 1 (borrow) = 0x1F
  cpu.alu().lhs().set_value(Byte{0x30});
  cpu.alu().rhs().set_value(Byte{0x10});
  cpu.status().carry().Set(false);  // Carry clear = borrow
  SetAluOpcode(cpu, 0x2);  // SUB

  test::SetPhase(cpu, irata2::base::TickPhase::Process);
  cpu.alu().TickProcess();

  EXPECT_EQ(cpu.alu().result().value(), Byte{0x1F});
}

// ============================================================================
// No-Op Tests (Opcode 0x0)
// ============================================================================

TEST(AluTest, NoOpcode) {
  Cpu cpu = test::MakeTestCpu();

  // Setup: no opcode bits set
  cpu.alu().lhs().set_value(Byte{0x10});
  cpu.alu().rhs().set_value(Byte{0x20});
  cpu.alu().result().set_value(Byte{0xFF});  // Initial value

  test::SetPhase(cpu, irata2::base::TickPhase::Process);
  cpu.alu().TickProcess();

  // Result should be unchanged when opcode is 0
  EXPECT_EQ(cpu.alu().result().value(), Byte{0xFF});
}
