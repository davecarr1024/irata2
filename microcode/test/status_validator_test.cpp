#include "irata2/microcode/compiler/status_validator.h"

#include "irata2/hdl.h"
#include "irata2/microcode/encoder/status_encoder.h"
#include "irata2/microcode/error.h"
#include "irata2/microcode/output/program.h"

#include <gtest/gtest.h>

using irata2::hdl::ControlInfo;
using irata2::hdl::Cpu;
using irata2::microcode::MicrocodeError;
using irata2::microcode::compiler::StatusValidator;
using irata2::microcode::encoder::StatusEncoder;
using irata2::microcode::ir::Instruction;
using irata2::microcode::ir::InstructionSet;
using irata2::microcode::ir::InstructionVariant;
using irata2::microcode::ir::Step;
using irata2::microcode::output::StatusBitDefinition;
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

InstructionVariant MakeVariant(std::map<std::string, bool> status_conditions,
                               std::vector<Step> steps) {
  InstructionVariant variant;
  variant.status_conditions = std::move(status_conditions);
  variant.steps = std::move(steps);
  return variant;
}
}  // namespace

TEST(StatusValidatorTest, AcceptsUnconditionalNoVariants) {
  Cpu cpu;
  std::vector<StatusBitDefinition> status_bits = {
      {"zero", 0},
  };
  StatusEncoder encoder(status_bits);
  StatusValidator validator(encoder);

  InstructionSet set;
  // Instruction with no variants (unusual but valid)
  set.instructions.push_back(MakeInstruction(Opcode::HLT_IMP, {}));

  EXPECT_NO_THROW(validator.Run(set));
}

TEST(StatusValidatorTest, AcceptsUnconditionalSingleVariant) {
  Cpu cpu;
  std::vector<StatusBitDefinition> status_bits = {
      {"zero", 0},
  };
  StatusEncoder encoder(status_bits);
  StatusValidator validator(encoder);

  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP,
      {MakeVariant({}, {MakeStep(0, {&cpu.halt().control_info()})})}));

  EXPECT_NO_THROW(validator.Run(set));
}

TEST(StatusValidatorTest, AcceptsComplementarySingleBitVariants) {
  Cpu cpu;
  std::vector<StatusBitDefinition> status_bits = {
      {"zero", 0},
  };
  StatusEncoder encoder(status_bits);
  StatusValidator validator(encoder);

  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::JEQ_ABS,
      {MakeVariant({{"zero", true}}, {MakeStep(0, {&cpu.a().read().control_info()})}),
       MakeVariant({{"zero", false}}, {MakeStep(0, {&cpu.x().read().control_info()})})}));

  EXPECT_NO_THROW(validator.Run(set));
}

TEST(StatusValidatorTest, AcceptsTwoBitCompleteCoverage) {
  Cpu cpu;
  std::vector<StatusBitDefinition> status_bits = {
      {"zero", 0},
      {"carry", 1},
  };
  StatusEncoder encoder(status_bits);
  StatusValidator validator(encoder);

  InstructionSet set;
  // Four variants to cover all combinations of 2 bits
  set.instructions.push_back(MakeInstruction(
      Opcode::JEQ_ABS,
      {MakeVariant({{"zero", false}, {"carry", false}}, {MakeStep(0, {})}),
       MakeVariant({{"zero", false}, {"carry", true}}, {MakeStep(0, {})}),
       MakeVariant({{"zero", true}, {"carry", false}}, {MakeStep(0, {})}),
       MakeVariant({{"zero", true}, {"carry", true}}, {MakeStep(0, {})})}));

  // This violates the single-status rule, so it should fail
  EXPECT_THROW(validator.Run(set), MicrocodeError);
}

TEST(StatusValidatorTest, RejectsMultipleStatusBits) {
  Cpu cpu;
  std::vector<StatusBitDefinition> status_bits = {
      {"zero", 0},
      {"carry", 1},
  };
  StatusEncoder encoder(status_bits);
  StatusValidator validator(encoder);

  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::JEQ_ABS,
      {MakeVariant({{"zero", true}, {"carry", true}}, {MakeStep(0, {})})}));

  EXPECT_THROW(validator.Run(set), MicrocodeError);
}

TEST(StatusValidatorTest, RejectsIncompleteCoverage) {
  Cpu cpu;
  std::vector<StatusBitDefinition> status_bits = {
      {"zero", 0},
  };
  StatusEncoder encoder(status_bits);
  StatusValidator validator(encoder);

  InstructionSet set;
  // Only zero: true, missing zero: false
  set.instructions.push_back(MakeInstruction(
      Opcode::JEQ_ABS,
      {MakeVariant({{"zero", true}}, {MakeStep(0, {})})}));

  EXPECT_THROW(validator.Run(set), MicrocodeError);
}

TEST(StatusValidatorTest, RejectsOverlappingVariants) {
  Cpu cpu;
  std::vector<StatusBitDefinition> status_bits = {
      {"zero", 0},
  };
  StatusEncoder encoder(status_bits);
  StatusValidator validator(encoder);

  InstructionSet set;
  // Both variants specify zero: true (overlap)
  set.instructions.push_back(MakeInstruction(
      Opcode::JEQ_ABS,
      {MakeVariant({{"zero", true}}, {MakeStep(0, {})}),
       MakeVariant({{"zero", true}}, {MakeStep(0, {})})}));

  EXPECT_THROW(validator.Run(set), MicrocodeError);
}

TEST(StatusValidatorTest, AcceptsNoStatusBitsConfigured) {
  Cpu cpu;
  std::vector<StatusBitDefinition> status_bits = {};  // No status bits
  StatusEncoder encoder(status_bits);
  StatusValidator validator(encoder);

  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP,
      {MakeVariant({}, {MakeStep(0, {&cpu.halt().control_info()})})}));

  EXPECT_NO_THROW(validator.Run(set));
}
