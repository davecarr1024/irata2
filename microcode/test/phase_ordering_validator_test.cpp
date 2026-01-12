#include "irata2/microcode/compiler/phase_ordering_validator.h"

#include "irata2/hdl.h"
#include "irata2/microcode/error.h"

#include <gtest/gtest.h>

using irata2::hdl::ControlInfo;
using irata2::hdl::Cpu;
using irata2::microcode::MicrocodeError;
using irata2::microcode::compiler::PhaseOrderingValidator;
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

TEST(PhaseOrderingValidatorTest, AcceptsValidReadWritePhases) {
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP,
      {MakeStep(0, {&cpu.a().write().control_info(),
                    &cpu.x().read().control_info()})}));

  PhaseOrderingValidator validator;
  EXPECT_NO_THROW(validator.Run(set));
}

TEST(PhaseOrderingValidatorTest, AcceptsValidProcessPhases) {
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP,
      {MakeStep(0, {&cpu.pc().increment().control_info()})}));

  PhaseOrderingValidator validator;
  EXPECT_NO_THROW(validator.Run(set));
}

TEST(PhaseOrderingValidatorTest, AcceptsMultipleReadsAndWrites) {
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP,
      {MakeStep(0, {&cpu.pc().write().control_info(),
                    &cpu.memory().mar().read().control_info(),
                    &cpu.memory().write().control_info(),
                    &cpu.a().read().control_info()})}));

  PhaseOrderingValidator validator;
  EXPECT_NO_THROW(validator.Run(set));
}

TEST(PhaseOrderingValidatorTest, AcceptsMixedControlTypes) {
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP,
      {MakeStep(0, {&cpu.a().write().control_info(),
                    &cpu.x().read().control_info(),
                    &cpu.pc().increment().control_info(),
                    &cpu.halt().control_info()})}));

  PhaseOrderingValidator validator;
  EXPECT_NO_THROW(validator.Run(set));
}

TEST(PhaseOrderingValidatorTest, ValidatesFetchPreamble) {
  Cpu cpu;
  InstructionSet set;
  // Valid fetch preamble
  set.fetch_preamble.push_back(MakeStep(0, {&cpu.pc().write().control_info(),
                                              &cpu.memory().mar().read().control_info()}));

  PhaseOrderingValidator validator;
  EXPECT_NO_THROW(validator.Run(set));
}

TEST(PhaseOrderingValidatorTest, AcceptsStatusControls) {
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP,
      {MakeStep(0, {&cpu.status().carry().set().control_info(),
                    &cpu.status().zero().clear().control_info()})}));

  PhaseOrderingValidator validator;
  EXPECT_NO_THROW(validator.Run(set));
}

TEST(PhaseOrderingValidatorTest, AcceptsControllerIpcLatch) {
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP,
      {MakeStep(0, {&cpu.controller().ipc_latch().control_info()})}));

  PhaseOrderingValidator validator;
  EXPECT_NO_THROW(validator.Run(set));
}
