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
