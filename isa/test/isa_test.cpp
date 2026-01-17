#include "irata2/isa/isa.h"

#include <gtest/gtest.h>

using namespace irata2::isa;

TEST(IsaTest, AddressingModeToString) {
  const auto& modes = IsaInfo::GetAddressingModes();
  ASSERT_FALSE(modes.empty());

  for (const auto& mode : modes) {
    EXPECT_EQ(ToString(mode.mode), mode.name);
  }

  EXPECT_EQ(ToString(static_cast<AddressingMode>(0x7F)), "Unknown");
}

TEST(IsaTest, StatusFlagToString) {
  EXPECT_EQ(ToString(StatusFlag::Z), "Zero");
  EXPECT_EQ(ToString(StatusFlag::N), "Negative");
  EXPECT_EQ(ToString(StatusFlag::C), "Carry");
  EXPECT_EQ(ToString(StatusFlag::V), "Overflow");
  EXPECT_EQ(ToString(static_cast<StatusFlag>(0x7F)), "Unknown");
}

TEST(IsaTest, InstructionCategoryToString) {
  EXPECT_EQ(ToString(InstructionCategory::System), "System");
  EXPECT_EQ(ToString(static_cast<InstructionCategory>(0x7F)), "Unknown");
}

TEST(IsaTest, OpcodeToString) {
  const auto& instructions = IsaInfo::GetInstructions();
  ASSERT_FALSE(instructions.empty());

  for (const auto& inst : instructions) {
    EXPECT_NE(ToString(inst.opcode), "Unknown");
  }

  EXPECT_EQ(ToString(static_cast<Opcode>(0x00)), "Unknown");
}

TEST(IsaTest, GetAddressingModes) {
  const auto& modes = IsaInfo::GetAddressingModes();
  EXPECT_FALSE(modes.empty());

  bool found_imp = false;
  for (const auto& mode : modes) {
    if (mode.mode == AddressingMode::IMP) {
      found_imp = true;
      EXPECT_EQ(mode.name, "Implied");
      EXPECT_EQ(mode.code, "IMP");
      EXPECT_EQ(mode.operand_bytes, 0);
      break;
    }
  }
  EXPECT_TRUE(found_imp);
}

TEST(IsaTest, GetInstructions) {
  const auto& instructions = IsaInfo::GetInstructions();
  EXPECT_EQ(instructions.size(), 18u);
}

TEST(IsaTest, GetInstructionByOpcodeValue) {
  auto inst = IsaInfo::GetInstruction(0x01);
  ASSERT_TRUE(inst.has_value());

  EXPECT_EQ(inst->mnemonic, "HLT");
  EXPECT_EQ(inst->addressing_mode, AddressingMode::IMP);
  EXPECT_EQ(inst->category, InstructionCategory::System);
  EXPECT_EQ(inst->cycles, 1);
}

TEST(IsaTest, GetInstructionByOpcodeEnum) {
  auto inst = IsaInfo::GetInstruction(Opcode::HLT_IMP);
  ASSERT_TRUE(inst.has_value());

  EXPECT_EQ(inst->mnemonic, "HLT");
  EXPECT_EQ(inst->addressing_mode, AddressingMode::IMP);
}

TEST(IsaTest, GetInstructionInvalidOpcode) {
  auto inst = IsaInfo::GetInstruction(0x00);
  EXPECT_FALSE(inst.has_value());
}

TEST(IsaTest, GetInstructionForAllOpcodes) {
  const auto& instructions = IsaInfo::GetInstructions();
  ASSERT_FALSE(instructions.empty());

  for (const auto& inst : instructions) {
    auto by_value = IsaInfo::GetInstruction(static_cast<uint8_t>(inst.opcode));
    ASSERT_TRUE(by_value.has_value());
    EXPECT_EQ(by_value->opcode, inst.opcode);

    auto by_enum = IsaInfo::GetInstruction(inst.opcode);
    ASSERT_TRUE(by_enum.has_value());
    EXPECT_EQ(by_enum->mnemonic, inst.mnemonic);
  }
}

TEST(IsaTest, GetAddressingModeByEnum) {
  auto mode = IsaInfo::GetAddressingMode(AddressingMode::IMP);
  ASSERT_TRUE(mode.has_value());

  EXPECT_EQ(mode->name, "Implied");
  EXPECT_EQ(mode->code, "IMP");
  EXPECT_EQ(mode->operand_bytes, 0);
  EXPECT_EQ(mode->syntax, "");
}

TEST(IsaTest, GetAddressingModeInvalidEnum) {
  auto mode = IsaInfo::GetAddressingMode(static_cast<AddressingMode>(0x7F));
  EXPECT_FALSE(mode.has_value());
}

TEST(IsaTest, InstructionFlagsAffected) {
  auto inst = IsaInfo::GetInstruction(Opcode::NOP_IMP);
  ASSERT_TRUE(inst.has_value());

  EXPECT_TRUE(inst->flags_affected.empty());
}

TEST(IsaTest, SystemInstructions) {
  auto inst = IsaInfo::GetInstruction(Opcode::HLT_IMP);
  ASSERT_TRUE(inst.has_value());

  EXPECT_EQ(inst->category, InstructionCategory::System);
  EXPECT_EQ(inst->addressing_mode, AddressingMode::IMP);
  EXPECT_TRUE(inst->flags_affected.empty());
}
