#include "irata2/microcode/compiler/duplicate_step_optimizer.h"

#include "irata2/hdl.h"

#include <gtest/gtest.h>

using irata2::hdl::ControlInfo;
using irata2::hdl::Cpu;
using irata2::microcode::compiler::DuplicateStepOptimizer;
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

TEST(DuplicateStepOptimizerTest, RemovesDuplicateAdjacentSteps) {
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP,
      {MakeVariant({MakeStep(0, {&cpu.halt().control_info()}),
                    MakeStep(0, {&cpu.halt().control_info()}),  // Duplicate
                    MakeStep(1, {&cpu.crash().control_info()})})}));

  DuplicateStepOptimizer optimizer;
  optimizer.Run(set);

  ASSERT_EQ(set.instructions.size(), 1);
  ASSERT_EQ(set.instructions[0].variants.size(), 1);
  ASSERT_EQ(set.instructions[0].variants[0].steps.size(), 2);
  EXPECT_EQ(set.instructions[0].variants[0].steps[0].stage, 0);
  EXPECT_EQ(set.instructions[0].variants[0].steps[1].stage, 1);
}

TEST(DuplicateStepOptimizerTest, PreservesNonDuplicateSteps) {
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP,
      {MakeVariant({MakeStep(0, {&cpu.halt().control_info()}),
                    MakeStep(1, {&cpu.crash().control_info()})})}));

  DuplicateStepOptimizer optimizer;
  optimizer.Run(set);

  ASSERT_EQ(set.instructions.size(), 1);
  ASSERT_EQ(set.instructions[0].variants.size(), 1);
  EXPECT_EQ(set.instructions[0].variants[0].steps.size(), 2);
}

TEST(DuplicateStepOptimizerTest, RemovesMultipleConsecutiveDuplicates) {
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP,
      {MakeVariant({MakeStep(0, {&cpu.halt().control_info()}),
                    MakeStep(0, {&cpu.halt().control_info()}),  // Duplicate
                    MakeStep(0, {&cpu.halt().control_info()}),  // Duplicate
                    MakeStep(1, {&cpu.crash().control_info()})})}));

  DuplicateStepOptimizer optimizer;
  optimizer.Run(set);

  ASSERT_EQ(set.instructions.size(), 1);
  ASSERT_EQ(set.instructions[0].variants.size(), 1);
  ASSERT_EQ(set.instructions[0].variants[0].steps.size(), 2);
}

TEST(DuplicateStepOptimizerTest, PreservesNonAdjacentIdenticalSteps) {
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP,
      {MakeVariant({MakeStep(0, {&cpu.halt().control_info()}),
                    MakeStep(1, {&cpu.crash().control_info()}),
                    MakeStep(2, {&cpu.halt().control_info()})})}));  // Same as step 0

  DuplicateStepOptimizer optimizer;
  optimizer.Run(set);

  ASSERT_EQ(set.instructions.size(), 1);
  ASSERT_EQ(set.instructions[0].variants.size(), 1);
  EXPECT_EQ(set.instructions[0].variants[0].steps.size(), 3);
}

TEST(DuplicateStepOptimizerTest, HandlesDifferentControlsInSameStage) {
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP,
      {MakeVariant({MakeStep(0, {&cpu.halt().control_info()}),
                    MakeStep(0, {&cpu.crash().control_info()})})}));

  DuplicateStepOptimizer optimizer;
  optimizer.Run(set);

  ASSERT_EQ(set.instructions.size(), 1);
  ASSERT_EQ(set.instructions[0].variants.size(), 1);
  EXPECT_EQ(set.instructions[0].variants[0].steps.size(), 2);
}

TEST(DuplicateStepOptimizerTest, HandlesDifferentStagesWithSameControls) {
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP,
      {MakeVariant({MakeStep(0, {&cpu.halt().control_info()}),
                    MakeStep(1, {&cpu.halt().control_info()})})}));

  DuplicateStepOptimizer optimizer;
  optimizer.Run(set);

  ASSERT_EQ(set.instructions.size(), 1);
  ASSERT_EQ(set.instructions[0].variants.size(), 1);
  // Different stages - should NOT be merged
  EXPECT_EQ(set.instructions[0].variants[0].steps.size(), 2);
}

TEST(DuplicateStepOptimizerTest, HandlesEmptyVariant) {
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(
      MakeInstruction(Opcode::NOP_IMP, {MakeVariant({})}));

  DuplicateStepOptimizer optimizer;
  optimizer.Run(set);

  ASSERT_EQ(set.instructions.size(), 1);
  ASSERT_EQ(set.instructions[0].variants.size(), 1);
  EXPECT_EQ(set.instructions[0].variants[0].steps.size(), 0);
}

TEST(DuplicateStepOptimizerTest, HandlesSingleStep) {
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP,
      {MakeVariant({MakeStep(0, {&cpu.halt().control_info()})})}));

  DuplicateStepOptimizer optimizer;
  optimizer.Run(set);

  ASSERT_EQ(set.instructions.size(), 1);
  ASSERT_EQ(set.instructions[0].variants.size(), 1);
  EXPECT_EQ(set.instructions[0].variants[0].steps.size(), 1);
}

TEST(DuplicateStepOptimizerTest, HandlesMultipleVariants) {
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::JEQ_ABS,
      {MakeVariant({MakeStep(0, {&cpu.halt().control_info()}),
                    MakeStep(0, {&cpu.halt().control_info()})}),  // Duplicate
       MakeVariant({MakeStep(0, {&cpu.crash().control_info()}),
                    MakeStep(0, {&cpu.crash().control_info()})})}));  // Duplicate

  DuplicateStepOptimizer optimizer;
  optimizer.Run(set);

  ASSERT_EQ(set.instructions.size(), 1);
  ASSERT_EQ(set.instructions[0].variants.size(), 2);
  EXPECT_EQ(set.instructions[0].variants[0].steps.size(), 1);
  EXPECT_EQ(set.instructions[0].variants[1].steps.size(), 1);
}

TEST(DuplicateStepOptimizerTest, OptimizesFetchPreamble) {
  Cpu cpu;
  InstructionSet set;
  set.fetch_preamble.push_back(MakeStep(0, {&cpu.halt().control_info()}));
  set.fetch_preamble.push_back(MakeStep(0, {&cpu.halt().control_info()}));  // Duplicate
  set.fetch_preamble.push_back(MakeStep(1, {&cpu.crash().control_info()}));

  DuplicateStepOptimizer optimizer;
  optimizer.Run(set);

  ASSERT_EQ(set.fetch_preamble.size(), 2);
  EXPECT_EQ(set.fetch_preamble[0].stage, 0);
  EXPECT_EQ(set.fetch_preamble[1].stage, 1);
}

TEST(DuplicateStepOptimizerTest, HandlesDifferentControlCounts) {
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP,
      {MakeVariant({MakeStep(0, {&cpu.halt().control_info()}),
                    MakeStep(0, {&cpu.halt().control_info(),
                                 &cpu.crash().control_info()})})}));

  DuplicateStepOptimizer optimizer;
  optimizer.Run(set);

  ASSERT_EQ(set.instructions.size(), 1);
  ASSERT_EQ(set.instructions[0].variants.size(), 1);
  // Different control counts - should NOT be merged
  EXPECT_EQ(set.instructions[0].variants[0].steps.size(), 2);
}
