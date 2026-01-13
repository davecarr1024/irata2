#include "irata2/microcode/compiler/control_conflict_validator.h"

#include "irata2/hdl.h"
#include "irata2/microcode/error.h"

#include <gtest/gtest.h>

using irata2::hdl::ControlInfo;
using irata2::hdl::Cpu;
using irata2::microcode::MicrocodeError;
using irata2::microcode::compiler::ControlConflictValidator;
using irata2::microcode::ir::Instruction;
using irata2::microcode::ir::InstructionSet;
using irata2::microcode::ir::InstructionVariant;
using irata2::microcode::ir::Step;
using irata2::isa::Opcode;

namespace {
Step MakeStep(int stage,
              std::initializer_list<const ControlInfo*> controls) {
  Step step;
  step.stage = stage;
  step.controls = {controls.begin(), controls.end()};
  return step;
}

Instruction MakeInstruction(Opcode opcode, std::vector<Step> steps) {
  Instruction instruction;
  instruction.opcode = opcode;
  InstructionVariant variant;
  variant.steps = std::move(steps);
  instruction.variants.push_back(std::move(variant));
  return instruction;
}
}  // namespace

TEST(ControlConflictValidatorTest, AcceptsValidReadWriteDifferentComponents) {
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP,
      {MakeStep(0, {&cpu.a().write().control_info(),
                    &cpu.x().read().control_info()})}));

  ControlConflictValidator validator;
  EXPECT_NO_THROW(validator.Run(set));
}

TEST(ControlConflictValidatorTest, AcceptsSingleAluOpcodeBit) {
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP,
      {MakeStep(0, {&cpu.alu().opcode_bit_0().control_info()})}));

  ControlConflictValidator validator;
  EXPECT_NO_THROW(validator.Run(set));
}

TEST(ControlConflictValidatorTest, AcceptsSingleStatusOperation) {
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP,
      {MakeStep(0, {&cpu.status().zero().set().control_info()})}));

  ControlConflictValidator validator;
  EXPECT_NO_THROW(validator.Run(set));
}

TEST(ControlConflictValidatorTest, AcceptsIncrementOnly) {
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP,
      {MakeStep(0, {&cpu.pc().increment().control_info()})}));

  ControlConflictValidator validator;
  EXPECT_NO_THROW(validator.Run(set));
}

TEST(ControlConflictValidatorTest, RejectsReadWriteSameComponent) {
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP,
      {MakeStep(0, {&cpu.a().read().control_info(),
                    &cpu.a().write().control_info()})}));

  ControlConflictValidator validator;
  EXPECT_THROW(validator.Run(set), MicrocodeError);
}

TEST(ControlConflictValidatorTest, AcceptsMultipleAluOpcodeBits) {
  // ALU opcode is binary encoded, so multiple bits being set is valid
  // (they form a single opcode value, not conflicting operations)
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP,
      {MakeStep(0, {&cpu.alu().opcode_bit_0().control_info(),
                    &cpu.alu().opcode_bit_1().control_info()})}));

  ControlConflictValidator validator;
  EXPECT_NO_THROW(validator.Run(set));
}

TEST(ControlConflictValidatorTest, RejectsSetClearSameFlag) {
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP,
      {MakeStep(0, {&cpu.status().zero().set().control_info(),
                    &cpu.status().zero().clear().control_info()})}));

  ControlConflictValidator validator;
  EXPECT_THROW(validator.Run(set), MicrocodeError);
}

TEST(ControlConflictValidatorTest, AcceptsDifferentStatusFlags) {
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP,
      {MakeStep(0, {&cpu.status().zero().set().control_info(),
                    &cpu.status().carry().clear().control_info()})}));

  ControlConflictValidator validator;
  EXPECT_NO_THROW(validator.Run(set));
}

TEST(ControlConflictValidatorTest, ValidatesFetchPreamble) {
  Cpu cpu;
  InstructionSet set;
  // Valid fetch preamble
  set.fetch_preamble.push_back(MakeStep(0, {&cpu.pc().write().control_info(),
                                              &cpu.memory().mar().read().control_info()}));

  ControlConflictValidator validator;
  EXPECT_NO_THROW(validator.Run(set));
}
