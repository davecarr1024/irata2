#include "irata2/microcode/compiler/sequence_validator.h"

#include "irata2/hdl.h"
#include "irata2/microcode/error.h"

#include <gtest/gtest.h>

using irata2::hdl::Cpu;
using irata2::microcode::MicrocodeError;
using irata2::microcode::compiler::SequenceValidator;
using irata2::microcode::ir::Instruction;
using irata2::microcode::ir::InstructionSet;
using irata2::microcode::ir::InstructionVariant;
using irata2::microcode::ir::Step;
using irata2::isa::Opcode;

namespace {
Step MakeStep(int stage, std::initializer_list<const irata2::hdl::ControlBase*> controls) {
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

TEST(SequenceValidatorTest, AcceptsIncrementAndReset) {
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP,
      {MakeStep(1, {&cpu.controller().sc().increment()}),
       MakeStep(1, {&cpu.controller().sc().reset()})}));

  SequenceValidator validator(cpu.controller().sc().increment(),
                              cpu.controller().sc().reset());
  EXPECT_NO_THROW(validator.Run(set));
}

TEST(SequenceValidatorTest, RejectsMissingIncrement) {
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP,
      {MakeStep(1, {&cpu.halt()}),
       MakeStep(1, {&cpu.controller().sc().reset()})}));

  SequenceValidator validator(cpu.controller().sc().increment(),
                              cpu.controller().sc().reset());
  EXPECT_THROW(validator.Run(set), MicrocodeError);
}

TEST(SequenceValidatorTest, RejectsMissingReset) {
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::NOP_IMP,
      {MakeStep(1, {&cpu.controller().sc().increment()}),
       MakeStep(1, {&cpu.halt()})}));

  SequenceValidator validator(cpu.controller().sc().increment(),
                              cpu.controller().sc().reset());
  EXPECT_THROW(validator.Run(set), MicrocodeError);
}

TEST(SequenceValidatorTest, SkipsEmptyVariants) {
  Cpu cpu;
  InstructionSet set;
  Instruction instruction;
  instruction.opcode = Opcode::CRS_IMP;
  InstructionVariant variant;
  instruction.variants.push_back(std::move(variant));
  set.instructions.push_back(std::move(instruction));

  SequenceValidator validator(cpu.controller().sc().increment(),
                              cpu.controller().sc().reset());
  EXPECT_NO_THROW(validator.Run(set));
}
