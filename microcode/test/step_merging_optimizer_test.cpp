#include "irata2/microcode/compiler/step_merging_optimizer.h"

#include "irata2/hdl.h"

#include <gtest/gtest.h>

using irata2::hdl::Cpu;
using irata2::microcode::compiler::StepMergingOptimizer;
using irata2::microcode::ir::Instruction;
using irata2::microcode::ir::InstructionSet;
using irata2::microcode::ir::InstructionVariant;
using irata2::microcode::ir::Step;
using irata2::isa::Opcode;

namespace {

Step MakeStep(int stage,
              std::initializer_list<const irata2::hdl::ControlInfo*> controls) {
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

TEST(StepMergingOptimizerTest, MergesWriteBeforeRead) {
  // Write phase (2) < Read phase (3) - should merge
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP,
      {MakeStep(0, {&cpu.pc().write().control_info()}),       // Write phase
       MakeStep(0, {&cpu.memory().mar().read().control_info()})}));  // Read phase

  StepMergingOptimizer optimizer;
  optimizer.Run(set);

  ASSERT_EQ(set.instructions[0].variants[0].steps.size(), 1u);
  EXPECT_EQ(set.instructions[0].variants[0].steps[0].controls.size(), 2u);
}

TEST(StepMergingOptimizerTest, MergesReadBeforeProcess) {
  // Read phase (3) < Process phase (4) - should merge
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP,
      {MakeStep(0, {&cpu.a().read().control_info()}),    // Read phase
       MakeStep(0, {&cpu.a().reset().control_info()})}));  // Process phase

  StepMergingOptimizer optimizer;
  optimizer.Run(set);

  ASSERT_EQ(set.instructions[0].variants[0].steps.size(), 1u);
  EXPECT_EQ(set.instructions[0].variants[0].steps[0].controls.size(), 2u);
}

TEST(StepMergingOptimizerTest, DoesNotMergeReadBeforeWrite) {
  // Read phase (3) > Write phase (2) - should NOT merge
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP,
      {MakeStep(0, {&cpu.a().read().control_info()}),     // Read phase
       MakeStep(0, {&cpu.pc().write().control_info()})}));  // Write phase

  StepMergingOptimizer optimizer;
  optimizer.Run(set);

  EXPECT_EQ(set.instructions[0].variants[0].steps.size(), 2u);
}

TEST(StepMergingOptimizerTest, DoesNotMergeProcessBeforeRead) {
  // Process phase (4) > Read phase (3) - should NOT merge
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP,
      {MakeStep(0, {&cpu.a().reset().control_info()}),   // Process phase
       MakeStep(0, {&cpu.a().read().control_info()})}));   // Read phase

  StepMergingOptimizer optimizer;
  optimizer.Run(set);

  EXPECT_EQ(set.instructions[0].variants[0].steps.size(), 2u);
}

TEST(StepMergingOptimizerTest, DoesNotMergeAcrossStages) {
  // Same phase but different stages - should NOT merge
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP,
      {MakeStep(0, {&cpu.pc().write().control_info()}),   // Stage 0
       MakeStep(1, {&cpu.a().write().control_info()})}));   // Stage 1

  StepMergingOptimizer optimizer;
  optimizer.Run(set);

  EXPECT_EQ(set.instructions[0].variants[0].steps.size(), 2u);
}

TEST(StepMergingOptimizerTest, DoesNotMergeSamePhase) {
  // Same phase (Write) and same stage - should NOT merge
  // (merging same-phase controls changes semantics)
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP,
      {MakeStep(0, {&cpu.pc().write().control_info()}),
       MakeStep(0, {&cpu.a().write().control_info()})}));

  StepMergingOptimizer optimizer;
  optimizer.Run(set);

  EXPECT_EQ(set.instructions[0].variants[0].steps.size(), 2u);
}

TEST(StepMergingOptimizerTest, MergesMultipleSteps) {
  // Write < Read < Process - all three should merge into one
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP,
      {MakeStep(0, {&cpu.pc().write().control_info()}),         // Write
       MakeStep(0, {&cpu.memory().mar().read().control_info()}),  // Read
       MakeStep(0, {&cpu.a().reset().control_info()})}));         // Process

  StepMergingOptimizer optimizer;
  optimizer.Run(set);

  ASSERT_EQ(set.instructions[0].variants[0].steps.size(), 1u);
  EXPECT_EQ(set.instructions[0].variants[0].steps[0].controls.size(), 3u);
}

TEST(StepMergingOptimizerTest, PartialMerge) {
  // Write, Read, Write - first two merge, third doesn't
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP,
      {MakeStep(0, {&cpu.pc().write().control_info()}),         // Write
       MakeStep(0, {&cpu.memory().mar().read().control_info()}),  // Read (merges with above)
       MakeStep(0, {&cpu.a().write().control_info()})}));         // Write (can't merge)

  StepMergingOptimizer optimizer;
  optimizer.Run(set);

  ASSERT_EQ(set.instructions[0].variants[0].steps.size(), 2u);
  EXPECT_EQ(set.instructions[0].variants[0].steps[0].controls.size(), 2u);
  EXPECT_EQ(set.instructions[0].variants[0].steps[1].controls.size(), 1u);
}

TEST(StepMergingOptimizerTest, PreservesEmptyVariant) {
  InstructionSet set;
  Instruction instruction;
  instruction.opcode = Opcode::HLT_IMP;
  instruction.variants.push_back({});  // Empty variant
  set.instructions.push_back(std::move(instruction));

  StepMergingOptimizer optimizer;
  EXPECT_NO_THROW(optimizer.Run(set));
  EXPECT_EQ(set.instructions[0].variants[0].steps.size(), 0u);
}

TEST(StepMergingOptimizerTest, PreservesSingleStep) {
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP,
      {MakeStep(0, {&cpu.pc().write().control_info()})}));

  StepMergingOptimizer optimizer;
  optimizer.Run(set);

  EXPECT_EQ(set.instructions[0].variants[0].steps.size(), 1u);
}

TEST(StepMergingOptimizerTest, OptimizesFetchPreamble) {
  Cpu cpu;
  InstructionSet set;
  // Fetch preamble with mergeable steps
  set.fetch_preamble = {
      MakeStep(0, {&cpu.pc().write().control_info()}),
      MakeStep(0, {&cpu.memory().mar().read().control_info()})};

  StepMergingOptimizer optimizer;
  optimizer.Run(set);

  ASSERT_EQ(set.fetch_preamble.size(), 1u);
  EXPECT_EQ(set.fetch_preamble[0].controls.size(), 2u);
}

TEST(StepMergingOptimizerTest, DoesNotMergeWhenSharingControl) {
  // If the same control appears in both steps, they can't merge because
  // they have the same phase (strict ordering requires max(a) < min(b))
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP,
      {MakeStep(0, {&cpu.pc().write().control_info()}),
       MakeStep(0, {&cpu.pc().write().control_info(),
                    &cpu.memory().mar().read().control_info()})}));

  StepMergingOptimizer optimizer;
  optimizer.Run(set);

  // Steps sharing pc.write (Write phase) can't merge - Write < Write is false
  ASSERT_EQ(set.instructions[0].variants[0].steps.size(), 2u);
  EXPECT_EQ(set.instructions[0].variants[0].steps[0].controls.size(), 1u);
  EXPECT_EQ(set.instructions[0].variants[0].steps[1].controls.size(), 2u);
}
