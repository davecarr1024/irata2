#include "irata2/microcode/compiler/empty_step_optimizer.h"

#include "irata2/hdl.h"

#include <gtest/gtest.h>

using irata2::hdl::ControlInfo;
using irata2::hdl::Cpu;
using irata2::microcode::compiler::EmptyStepOptimizer;
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

Instruction MakeInstruction(Opcode opcode,
                           std::vector<InstructionVariant> variants) {
  Instruction instruction;
  instruction.opcode = opcode;
  instruction.variants = std::move(variants);
  return instruction;
}

InstructionVariant MakeVariant(std::vector<Step> steps) {
  InstructionVariant variant;
  variant.steps = std::move(steps);
  return variant;
}

}  // namespace

TEST(EmptyStepOptimizerTest, RemovesEmptyStepsFromFetchPreamble) {
  Cpu cpu;
  InstructionSet set;
  set.fetch_preamble.push_back(MakeStep(0, {&cpu.halt().control_info()}));
  set.fetch_preamble.push_back(MakeStep(1, {}));  // Empty
  set.fetch_preamble.push_back(MakeStep(2, {&cpu.crash().control_info()}));

  EmptyStepOptimizer optimizer;
  optimizer.Run(set);

  ASSERT_EQ(set.fetch_preamble.size(), 2);
  EXPECT_EQ(set.fetch_preamble[0].controls.size(), 1);
  EXPECT_EQ(set.fetch_preamble[1].controls.size(), 1);
}

TEST(EmptyStepOptimizerTest, RemovesEmptyStepsFromInstructions) {
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP,
      {MakeVariant({MakeStep(0, {&cpu.halt().control_info()}),
                    MakeStep(1, {}),  // Empty
                    MakeStep(2, {&cpu.crash().control_info()})})}));

  EmptyStepOptimizer optimizer;
  optimizer.Run(set);

  ASSERT_EQ(set.instructions.size(), 1);
  ASSERT_EQ(set.instructions[0].variants.size(), 1);
  ASSERT_EQ(set.instructions[0].variants[0].steps.size(), 2);
  EXPECT_EQ(set.instructions[0].variants[0].steps[0].controls.size(), 1);
  EXPECT_EQ(set.instructions[0].variants[0].steps[1].controls.size(), 1);
}

TEST(EmptyStepOptimizerTest, PreservesNonEmptySteps) {
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP,
      {MakeVariant({MakeStep(0, {&cpu.halt().control_info()}),
                    MakeStep(1, {&cpu.crash().control_info()})})}));

  EmptyStepOptimizer optimizer;
  optimizer.Run(set);

  ASSERT_EQ(set.instructions.size(), 1);
  ASSERT_EQ(set.instructions[0].variants.size(), 1);
  ASSERT_EQ(set.instructions[0].variants[0].steps.size(), 2);
}

TEST(EmptyStepOptimizerTest, HandlesAllEmptyVariant) {
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::NOP_IMP, {MakeVariant({MakeStep(0, {}), MakeStep(1, {})})}));

  EmptyStepOptimizer optimizer;
  optimizer.Run(set);

  ASSERT_EQ(set.instructions.size(), 1);
  ASSERT_EQ(set.instructions[0].variants.size(), 1);
  EXPECT_EQ(set.instructions[0].variants[0].steps.size(), 0);
}

TEST(EmptyStepOptimizerTest, HandlesMultipleVariants) {
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::JEQ_ABS,
      {MakeVariant({MakeStep(0, {&cpu.halt().control_info()}),
                    MakeStep(1, {}),  // Empty
                    MakeStep(2, {&cpu.crash().control_info()})}),
       MakeVariant({MakeStep(0, {}),  // Empty
                    MakeStep(1, {&cpu.a().read().control_info()})})}));

  EmptyStepOptimizer optimizer;
  optimizer.Run(set);

  ASSERT_EQ(set.instructions.size(), 1);
  ASSERT_EQ(set.instructions[0].variants.size(), 2);
  ASSERT_EQ(set.instructions[0].variants[0].steps.size(), 2);
  ASSERT_EQ(set.instructions[0].variants[1].steps.size(), 1);
}

TEST(EmptyStepOptimizerTest, RemovesConsecutiveEmptySteps) {
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP,
      {MakeVariant({MakeStep(0, {&cpu.halt().control_info()}),
                    MakeStep(1, {}),  // Empty
                    MakeStep(2, {}),  // Empty
                    MakeStep(3, {}),  // Empty
                    MakeStep(4, {&cpu.crash().control_info()})})}));

  EmptyStepOptimizer optimizer;
  optimizer.Run(set);

  ASSERT_EQ(set.instructions.size(), 1);
  ASSERT_EQ(set.instructions[0].variants.size(), 1);
  ASSERT_EQ(set.instructions[0].variants[0].steps.size(), 2);
}

TEST(EmptyStepOptimizerTest, PreservesStageNumbers) {
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP,
      {MakeVariant({MakeStep(0, {&cpu.halt().control_info()}),
                    MakeStep(1, {}),  // Empty
                    MakeStep(2, {&cpu.crash().control_info()})})}));

  EmptyStepOptimizer optimizer;
  optimizer.Run(set);

  ASSERT_EQ(set.instructions.size(), 1);
  ASSERT_EQ(set.instructions[0].variants.size(), 1);
  ASSERT_EQ(set.instructions[0].variants[0].steps.size(), 2);
  EXPECT_EQ(set.instructions[0].variants[0].steps[0].stage, 0);
  EXPECT_EQ(set.instructions[0].variants[0].steps[1].stage, 2);
}
