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
  EXPECT_EQ(ToString(InstructionCategory::Arithmetic), "Arithmetic");
  EXPECT_EQ(ToString(InstructionCategory::Branch), "Branch");
  EXPECT_EQ(ToString(InstructionCategory::Compare), "Compare");
  EXPECT_EQ(ToString(InstructionCategory::Jump), "Jump");
  EXPECT_EQ(ToString(InstructionCategory::Load), "Load");
  EXPECT_EQ(ToString(InstructionCategory::Logic), "Logic");
  EXPECT_EQ(ToString(InstructionCategory::Stack), "Stack");
  EXPECT_EQ(ToString(InstructionCategory::Store), "Store");
  EXPECT_EQ(ToString(InstructionCategory::System), "System");
  EXPECT_EQ(ToString(InstructionCategory::Transfer), "Transfer");
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

  // Find IMM mode
  bool found_imm = false;
  for (const auto& mode : modes) {
    if (mode.mode == AddressingMode::IMM) {
      found_imm = true;
      EXPECT_EQ(mode.name, "Immediate");
      EXPECT_EQ(mode.code, "IMM");
      EXPECT_EQ(mode.operand_bytes, 1);
      break;
    }
  }
  EXPECT_TRUE(found_imm);
}

TEST(IsaTest, GetInstructions) {
  const auto& instructions = IsaInfo::GetInstructions();
  EXPECT_FALSE(instructions.empty());

  // Count LDA instructions (should have multiple addressing modes)
  int lda_count = 0;
  for (const auto& inst : instructions) {
    if (inst.mnemonic == "LDA") {
      lda_count++;
    }
  }
  EXPECT_GT(lda_count, 1);  // Should have multiple LDA variants
}

TEST(IsaTest, GetInstructionByOpcodeValue) {
  // LDA_IMM is 0xA0
  auto inst = IsaInfo::GetInstruction(0xA0);
  ASSERT_TRUE(inst.has_value());

  EXPECT_EQ(inst->mnemonic, "LDA");
  EXPECT_EQ(inst->addressing_mode, AddressingMode::IMM);
  EXPECT_EQ(inst->category, InstructionCategory::Load);
  EXPECT_EQ(inst->cycles, 2);
}

TEST(IsaTest, GetInstructionByOpcodeEnum) {
  auto inst = IsaInfo::GetInstruction(Opcode::LDA_IMM);
  ASSERT_TRUE(inst.has_value());

  EXPECT_EQ(inst->mnemonic, "LDA");
  EXPECT_EQ(inst->addressing_mode, AddressingMode::IMM);
}

TEST(IsaTest, GetInstructionInvalidOpcode) {
  // 0x00 is not defined
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
  auto mode = IsaInfo::GetAddressingMode(AddressingMode::IMM);
  ASSERT_TRUE(mode.has_value());

  EXPECT_EQ(mode->name, "Immediate");
  EXPECT_EQ(mode->code, "IMM");
  EXPECT_EQ(mode->operand_bytes, 1);
  EXPECT_EQ(mode->syntax, "#$%02X");
}

TEST(IsaTest, GetAddressingModeInvalidEnum) {
  auto mode = IsaInfo::GetAddressingMode(static_cast<AddressingMode>(0x7F));
  EXPECT_FALSE(mode.has_value());
}

TEST(IsaTest, InstructionFlagsAffected) {
  // LDA affects Z and N flags
  auto inst = IsaInfo::GetInstruction(Opcode::LDA_IMM);
  ASSERT_TRUE(inst.has_value());

  ASSERT_EQ(inst->flags_affected.size(), 2);
  EXPECT_TRUE(std::find(inst->flags_affected.begin(), inst->flags_affected.end(),
                        StatusFlag::Z) != inst->flags_affected.end());
  EXPECT_TRUE(std::find(inst->flags_affected.begin(), inst->flags_affected.end(),
                        StatusFlag::N) != inst->flags_affected.end());
}

TEST(IsaTest, SystemInstructions) {
  // HLT is a system instruction
  auto inst = IsaInfo::GetInstruction(Opcode::HLT_IMP);
  ASSERT_TRUE(inst.has_value());

  EXPECT_EQ(inst->category, InstructionCategory::System);
  EXPECT_EQ(inst->addressing_mode, AddressingMode::IMP);
  EXPECT_TRUE(inst->flags_affected.empty());  // System instructions don't affect flags
}

TEST(IsaTest, ArithmeticInstructions) {
  // ADC affects Z, N, C, V flags
  auto inst = IsaInfo::GetInstruction(Opcode::ADC_IMM);
  ASSERT_TRUE(inst.has_value());

  EXPECT_EQ(inst->category, InstructionCategory::Arithmetic);
  EXPECT_EQ(inst->flags_affected.size(), 4);
}

TEST(IsaTest, BranchInstructions) {
  // BEQ is a branch instruction
  auto inst = IsaInfo::GetInstruction(Opcode::BEQ_REL);
  ASSERT_TRUE(inst.has_value());

  EXPECT_EQ(inst->category, InstructionCategory::Branch);
  EXPECT_EQ(inst->addressing_mode, AddressingMode::REL);
  EXPECT_TRUE(inst->flags_affected.empty());  // Branch instructions don't affect flags
}
