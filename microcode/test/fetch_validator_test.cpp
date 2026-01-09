#include "irata2/microcode/compiler/fetch_validator.h"

#include "irata2/hdl/cpu.h"
#include "irata2/microcode/error.h"

#include <gtest/gtest.h>

using irata2::hdl::Cpu;
using irata2::microcode::MicrocodeError;
using irata2::microcode::compiler::FetchValidator;
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

TEST(FetchValidatorTest, AcceptsMatchingPreamble) {
  Cpu cpu;
  InstructionSet set;
  set.fetch_preamble.push_back(MakeStep(0, {&cpu.a().read()}));
  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP,
      {MakeStep(0, {&cpu.a().read()}), MakeStep(1, {&cpu.halt()})}));

  FetchValidator validator;
  EXPECT_NO_THROW(validator.Run(set));
}

TEST(FetchValidatorTest, RejectsMismatchedPreambleSize) {
  Cpu cpu;
  InstructionSet set;
  set.fetch_preamble.push_back(MakeStep(0, {&cpu.a().read()}));
  set.fetch_preamble.push_back(MakeStep(0, {&cpu.a().write()}));
  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP,
      {MakeStep(0, {&cpu.a().read()}), MakeStep(1, {&cpu.halt()})}));

  FetchValidator validator;
  EXPECT_THROW(validator.Run(set), MicrocodeError);
}

TEST(FetchValidatorTest, RejectsMismatchedControls) {
  Cpu cpu;
  InstructionSet set;
  set.fetch_preamble.push_back(MakeStep(0, {&cpu.a().read()}));
  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP,
      {MakeStep(0, {&cpu.a().write()}), MakeStep(1, {&cpu.halt()})}));

  FetchValidator validator;
  EXPECT_THROW(validator.Run(set), MicrocodeError);
}

TEST(FetchValidatorTest, RejectsMismatchedControlCounts) {
  Cpu cpu;
  InstructionSet set;
  set.fetch_preamble.push_back(
      MakeStep(0, {&cpu.a().read(), &cpu.a().write()}));
  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP,
      {MakeStep(0, {&cpu.a().read()}), MakeStep(1, {&cpu.halt()})}));

  FetchValidator validator;
  EXPECT_THROW(validator.Run(set), MicrocodeError);
}

TEST(FetchValidatorTest, IgnoresWhenNoPreamble) {
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::NOP_IMP,
      {MakeStep(1, {&cpu.halt()})}));

  FetchValidator validator;
  EXPECT_NO_THROW(validator.Run(set));
}
