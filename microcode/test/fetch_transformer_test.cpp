#include "irata2/microcode/compiler/fetch_transformer.h"

#include "irata2/hdl/cpu.h"

#include <gtest/gtest.h>

using irata2::hdl::Cpu;
using irata2::microcode::compiler::FetchTransformer;
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

TEST(FetchTransformerTest, PrependsFetchPreambleAndRenumbersStages) {
  Cpu cpu;
  InstructionSet set;
  set.fetch_preamble.push_back(MakeStep(0, {&cpu.a().read()}));

  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP,
      {MakeStep(0, {&cpu.halt()}), MakeStep(1, {&cpu.crash()})}));

  FetchTransformer transformer;
  transformer.Run(set);

  const auto& steps = set.instructions.front().variants.front().steps;
  ASSERT_EQ(steps.size(), 3u);
  EXPECT_EQ(steps[0].stage, 0);
  EXPECT_EQ(steps[1].stage, 1);
  EXPECT_EQ(steps[2].stage, 2);
  EXPECT_EQ(steps[0].controls.front(), &cpu.a().read());
}

TEST(FetchTransformerTest, LeavesStagesWhenNoPreamble) {
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::NOP_IMP,
      {MakeStep(0, {&cpu.a().read()}), MakeStep(1, {&cpu.a().write()})}));

  FetchTransformer transformer;
  transformer.Run(set);

  const auto& steps = set.instructions.front().variants.front().steps;
  ASSERT_EQ(steps.size(), 2u);
  EXPECT_EQ(steps[0].stage, 0);
  EXPECT_EQ(steps[1].stage, 1);
}
