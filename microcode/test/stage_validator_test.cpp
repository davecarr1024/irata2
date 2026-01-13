#include "irata2/microcode/compiler/stage_validator.h"

#include "irata2/hdl.h"
#include "irata2/microcode/error.h"

#include <gtest/gtest.h>

using irata2::hdl::ControlInfo;
using irata2::hdl::Cpu;
using irata2::microcode::MicrocodeError;
using irata2::microcode::compiler::StageValidator;
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

TEST(StageValidatorTest, AcceptsSingleStage) {
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP,
      {MakeStep(0, {&cpu.halt().control_info()})}));

  StageValidator validator;
  EXPECT_NO_THROW(validator.Run(set));
}

TEST(StageValidatorTest, AcceptsMultipleStepsInSingleStage) {
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP,
      {MakeStep(0, {&cpu.a().read().control_info()}),
       MakeStep(0, {&cpu.x().read().control_info()}),
       MakeStep(0, {&cpu.halt().control_info()})}));

  StageValidator validator;
  EXPECT_NO_THROW(validator.Run(set));
}

TEST(StageValidatorTest, AcceptsMonotonicStages) {
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP,
      {MakeStep(0, {}),
       MakeStep(0, {}),
       MakeStep(1, {}),
       MakeStep(1, {}),
       MakeStep(2, {})}));

  StageValidator validator;
  EXPECT_NO_THROW(validator.Run(set));
}

TEST(StageValidatorTest, AcceptsSequentialStages) {
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP,
      {MakeStep(0, {}),
       MakeStep(1, {}),
       MakeStep(2, {}),
       MakeStep(3, {})}));

  StageValidator validator;
  EXPECT_NO_THROW(validator.Run(set));
}

TEST(StageValidatorTest, RejectsNotStartingAtZero) {
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP,
      {MakeStep(1, {}),
       MakeStep(2, {})}));

  StageValidator validator;
  EXPECT_THROW(validator.Run(set), MicrocodeError);
}

TEST(StageValidatorTest, RejectsNonMonotonic) {
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP,
      {MakeStep(0, {}),
       MakeStep(1, {}),
       MakeStep(0, {})}));

  StageValidator validator;
  EXPECT_THROW(validator.Run(set), MicrocodeError);
}

TEST(StageValidatorTest, RejectsGapInStages) {
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP,
      {MakeStep(0, {}),
       MakeStep(0, {}),
       MakeStep(2, {})}));  // Missing stage 1

  StageValidator validator;
  EXPECT_THROW(validator.Run(set), MicrocodeError);
}

TEST(StageValidatorTest, ValidatesFetchPreamble) {
  Cpu cpu;
  InstructionSet set;
  // Invalid: fetch preamble doesn't start at stage 0
  set.fetch_preamble.push_back(MakeStep(1, {}));

  StageValidator validator;
  EXPECT_THROW(validator.Run(set), MicrocodeError);
}

TEST(StageValidatorTest, AcceptsEmptyVariant) {
  Cpu cpu;
  InstructionSet set;
  Instruction instruction;
  instruction.opcode = Opcode::HLT_IMP;
  InstructionVariant variant;  // Empty steps
  instruction.variants.push_back(std::move(variant));
  set.instructions.push_back(std::move(instruction));

  StageValidator validator;
  EXPECT_NO_THROW(validator.Run(set));
}
