#include "irata2/microcode/compiler/isa_coverage_validator.h"

#include "irata2/microcode/error.h"

#include <gtest/gtest.h>

using irata2::microcode::MicrocodeError;
using irata2::microcode::compiler::IsaCoverageValidator;
using irata2::microcode::ir::Instruction;
using irata2::microcode::ir::InstructionSet;
using irata2::isa::Opcode;

namespace {
Instruction MakeInstruction(Opcode opcode) {
  Instruction instruction;
  instruction.opcode = opcode;
  return instruction;
}
}  // namespace

TEST(IsaCoverageValidatorTest, AcceptsExactCoverage) {
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(Opcode::HLT_IMP));
  set.instructions.push_back(MakeInstruction(Opcode::NOP_IMP));
  set.instructions.push_back(MakeInstruction(Opcode::CRS_IMP));
  set.instructions.push_back(MakeInstruction(Opcode::LDA_IMM));
  set.instructions.push_back(MakeInstruction(Opcode::CMP_IMM));
  set.instructions.push_back(MakeInstruction(Opcode::JEQ_ABS));
  set.instructions.push_back(MakeInstruction(Opcode::BEQ_REL));
  set.instructions.push_back(MakeInstruction(Opcode::BNE_REL));
  set.instructions.push_back(MakeInstruction(Opcode::BCS_REL));
  set.instructions.push_back(MakeInstruction(Opcode::BCC_REL));
  set.instructions.push_back(MakeInstruction(Opcode::BMI_REL));
  set.instructions.push_back(MakeInstruction(Opcode::BPL_REL));
  set.instructions.push_back(MakeInstruction(Opcode::BVS_REL));
  set.instructions.push_back(MakeInstruction(Opcode::BVC_REL));
  set.instructions.push_back(MakeInstruction(Opcode::ADC_IMM));
  set.instructions.push_back(MakeInstruction(Opcode::SBC_IMM));
  set.instructions.push_back(MakeInstruction(Opcode::AND_IMM));
  set.instructions.push_back(MakeInstruction(Opcode::ORA_IMM));
  set.instructions.push_back(MakeInstruction(Opcode::EOR_IMM));
  set.instructions.push_back(MakeInstruction(Opcode::ASL_IMP));
  set.instructions.push_back(MakeInstruction(Opcode::LSR_IMP));
  set.instructions.push_back(MakeInstruction(Opcode::ROL_IMP));
  set.instructions.push_back(MakeInstruction(Opcode::ROR_IMP));
  set.instructions.push_back(MakeInstruction(Opcode::TAX_IMP));
  set.instructions.push_back(MakeInstruction(Opcode::TXA_IMP));
  set.instructions.push_back(MakeInstruction(Opcode::LDX_IMM));
  set.instructions.push_back(MakeInstruction(Opcode::TAY_IMP));
  set.instructions.push_back(MakeInstruction(Opcode::TYA_IMP));
  set.instructions.push_back(MakeInstruction(Opcode::LDY_IMM));
  set.instructions.push_back(MakeInstruction(Opcode::LDA_ZP));
  set.instructions.push_back(MakeInstruction(Opcode::STA_ZP));
  set.instructions.push_back(MakeInstruction(Opcode::LDX_ZP));
  set.instructions.push_back(MakeInstruction(Opcode::STX_ZP));
  set.instructions.push_back(MakeInstruction(Opcode::LDY_ZP));
  set.instructions.push_back(MakeInstruction(Opcode::STY_ZP));
  set.instructions.push_back(MakeInstruction(Opcode::ADC_ZP));
  set.instructions.push_back(MakeInstruction(Opcode::SBC_ZP));
  set.instructions.push_back(MakeInstruction(Opcode::AND_ZP));
  set.instructions.push_back(MakeInstruction(Opcode::ORA_ZP));
  set.instructions.push_back(MakeInstruction(Opcode::EOR_ZP));
  set.instructions.push_back(MakeInstruction(Opcode::CMP_ZP));
  set.instructions.push_back(MakeInstruction(Opcode::ASL_ZP));
  set.instructions.push_back(MakeInstruction(Opcode::LSR_ZP));
  set.instructions.push_back(MakeInstruction(Opcode::ROL_ZP));
  set.instructions.push_back(MakeInstruction(Opcode::ROR_ZP));
  set.instructions.push_back(MakeInstruction(Opcode::LDA_ABS));
  set.instructions.push_back(MakeInstruction(Opcode::STA_ABS));
  set.instructions.push_back(MakeInstruction(Opcode::LDX_ABS));
  set.instructions.push_back(MakeInstruction(Opcode::STX_ABS));
  set.instructions.push_back(MakeInstruction(Opcode::LDY_ABS));
  set.instructions.push_back(MakeInstruction(Opcode::STY_ABS));
  set.instructions.push_back(MakeInstruction(Opcode::ADC_ABS));
  set.instructions.push_back(MakeInstruction(Opcode::SBC_ABS));
  set.instructions.push_back(MakeInstruction(Opcode::AND_ABS));
  set.instructions.push_back(MakeInstruction(Opcode::ORA_ABS));
  set.instructions.push_back(MakeInstruction(Opcode::EOR_ABS));
  set.instructions.push_back(MakeInstruction(Opcode::CMP_ABS));
  set.instructions.push_back(MakeInstruction(Opcode::ASL_ABS));
  set.instructions.push_back(MakeInstruction(Opcode::LSR_ABS));
  set.instructions.push_back(MakeInstruction(Opcode::ROL_ABS));
  set.instructions.push_back(MakeInstruction(Opcode::ROR_ABS));
  set.instructions.push_back(MakeInstruction(Opcode::INX_IMP));
  set.instructions.push_back(MakeInstruction(Opcode::DEX_IMP));
  set.instructions.push_back(MakeInstruction(Opcode::INY_IMP));
  set.instructions.push_back(MakeInstruction(Opcode::DEY_IMP));
  set.instructions.push_back(MakeInstruction(Opcode::INC_ZP));
  set.instructions.push_back(MakeInstruction(Opcode::DEC_ZP));
  set.instructions.push_back(MakeInstruction(Opcode::INC_ABS));
  set.instructions.push_back(MakeInstruction(Opcode::DEC_ABS));

  IsaCoverageValidator validator;
  EXPECT_NO_THROW(validator.Run(set));
}

TEST(IsaCoverageValidatorTest, RejectsMissingInstruction) {
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(Opcode::HLT_IMP));
  set.instructions.push_back(MakeInstruction(Opcode::NOP_IMP));

  IsaCoverageValidator validator;
  EXPECT_THROW(validator.Run(set), MicrocodeError);
}

TEST(IsaCoverageValidatorTest, RejectsDuplicateInstruction) {
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(Opcode::HLT_IMP));
  set.instructions.push_back(MakeInstruction(Opcode::HLT_IMP));
  set.instructions.push_back(MakeInstruction(Opcode::CRS_IMP));

  IsaCoverageValidator validator;
  EXPECT_THROW(validator.Run(set), MicrocodeError);
}

TEST(IsaCoverageValidatorTest, RejectsUnknownOpcode) {
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(Opcode::HLT_IMP));
  set.instructions.push_back(MakeInstruction(Opcode::NOP_IMP));
  set.instructions.push_back(MakeInstruction(static_cast<Opcode>(0x7F)));

  IsaCoverageValidator validator;
  EXPECT_THROW(validator.Run(set), MicrocodeError);
}
