#include "irata2/microcode/compiler/sequence_transformer.h"

#include "irata2/hdl.h"

#include <algorithm>
#include <gtest/gtest.h>

using irata2::hdl::Cpu;
using irata2::microcode::compiler::SequenceTransformer;
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

bool ContainsControl(const Step& step, const irata2::hdl::ControlBase& control) {
  return std::any_of(step.controls.begin(), step.controls.end(), [&](const auto* value) {
    return value == &control;
  });
}
}  // namespace

TEST(SequenceTransformerTest, AddsIncrementAndResetControls) {
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP,
      {MakeStep(1, {&cpu.halt()}), MakeStep(1, {&cpu.crash()})}));

  SequenceTransformer transformer(cpu.controller().sc().increment(),
                                  cpu.controller().sc().reset());
  transformer.Run(set);

  const auto& steps = set.instructions.front().variants.front().steps;
  ASSERT_EQ(steps.size(), 2u);
  EXPECT_TRUE(ContainsControl(steps[0], cpu.controller().sc().increment()));
  EXPECT_TRUE(ContainsControl(steps[1], cpu.controller().sc().reset()));
}

TEST(SequenceTransformerTest, DoesNotDuplicateControls) {
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::NOP_IMP,
      {MakeStep(0, {&cpu.controller().sc().increment()}),
       MakeStep(0, {&cpu.controller().sc().reset()})}));

  SequenceTransformer transformer(cpu.controller().sc().increment(),
                                  cpu.controller().sc().reset());
  transformer.Run(set);

  const auto& steps = set.instructions.front().variants.front().steps;
  EXPECT_EQ(steps[0].controls.size(), 1u);
  EXPECT_EQ(steps[1].controls.size(), 1u);
}

TEST(SequenceTransformerTest, SkipsEmptyVariants) {
  Cpu cpu;
  InstructionSet set;
  Instruction instruction;
  instruction.opcode = Opcode::CRS_IMP;
  InstructionVariant variant;
  instruction.variants.push_back(std::move(variant));
  set.instructions.push_back(std::move(instruction));

  SequenceTransformer transformer(cpu.controller().sc().increment(),
                                  cpu.controller().sc().reset());
  transformer.Run(set);

  EXPECT_TRUE(set.instructions.front().variants.front().steps.empty());
}
