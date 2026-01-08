#include "irata2/asm/isa.h"

#include <gtest/gtest.h>

using namespace irata2::isa;

TEST(IsaTest, AddressingModeToString) {
  EXPECT_EQ(ToString(AddressingMode::IMM), "Immediate");
  EXPECT_EQ(ToString(AddressingMode::ABS), "Absolute");
  EXPECT_EQ(ToString(AddressingMode::ZPG), "ZeroPage");
  EXPECT_EQ(ToString(AddressingMode::IMP), "Implied");
}

TEST(IsaTest, StatusFlagToString) {
  EXPECT_EQ(ToString(StatusFlag::Z), "Zero");
  EXPECT_EQ(ToString(StatusFlag::N), "Negative");
  EXPECT_EQ(ToString(StatusFlag::C), "Carry");
  EXPECT_EQ(ToString(StatusFlag::V), "Overflow");
}

TEST(IsaTest, InstructionCategoryToString) {
  EXPECT_EQ(ToString(InstructionCategory::Load), "Load");
  EXPECT_EQ(ToString(InstructionCategory::Store), "Store");
  EXPECT_EQ(ToString(InstructionCategory::Arithmetic), "Arithmetic");
  EXPECT_EQ(ToString(InstructionCategory::System), "System");
}

TEST(IsaTest, OpcodeToString) {
  EXPECT_EQ(ToString(Opcode::LDA_IMM), "LDA_IMM");
  EXPECT_EQ(ToString(Opcode::STA_ABS), "STA_ABS");
  EXPECT_EQ(ToString(Opcode::HLT_IMP), "HLT_IMP");
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

TEST(IsaTest, GetAddressingModeByEnum) {
  auto mode = IsaInfo::GetAddressingMode(AddressingMode::IMM);
  ASSERT_TRUE(mode.has_value());

  EXPECT_EQ(mode->name, "Immediate");
  EXPECT_EQ(mode->code, "IMM");
  EXPECT_EQ(mode->operand_bytes, 1);
  EXPECT_EQ(mode->syntax, "#$%02X");
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
