#include "irata2/microcode/compiler/bus_validator.h"

#include "irata2/hdl.h"
#include "irata2/microcode/error.h"

#include <gtest/gtest.h>

using irata2::hdl::ControlInfo;
using irata2::hdl::Cpu;
using irata2::microcode::MicrocodeError;
using irata2::microcode::compiler::BusValidator;
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

TEST(BusValidatorTest, AcceptsValidSingleWriterSingleReader) {
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP,
      {MakeStep(0, {&cpu.a().write().control_info(),
                    &cpu.x().read().control_info()})}));

  BusValidator validator;
  EXPECT_NO_THROW(validator.Run(set));
}

TEST(BusValidatorTest, AcceptsValidSingleWriterMultipleReaders) {
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP,
      {MakeStep(0, {&cpu.a().write().control_info(),
                    &cpu.x().read().control_info(),
                    &cpu.status().analyzer().read().control_info()})}));

  BusValidator validator;
  EXPECT_NO_THROW(validator.Run(set));
}

TEST(BusValidatorTest, AcceptsMultipleBuses) {
  Cpu cpu;
  InstructionSet set;
  // Data bus: a writes, x reads
  // Address bus: pc writes, memory.mar reads
  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP,
      {MakeStep(0, {&cpu.a().write().control_info(),
                    &cpu.x().read().control_info(),
                    &cpu.pc().write().control_info(),
                    &cpu.memory().mar().read().control_info()})}));

  BusValidator validator;
  EXPECT_NO_THROW(validator.Run(set));
}

TEST(BusValidatorTest, RejectsMultipleWritersDataBus) {
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP,
      {MakeStep(0, {&cpu.a().write().control_info(),
                    &cpu.x().write().control_info()})}));

  BusValidator validator;
  EXPECT_THROW(validator.Run(set), MicrocodeError);
}

TEST(BusValidatorTest, RejectsMultipleWritersAddressBus) {
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP,
      {MakeStep(0, {&cpu.pc().write().control_info(),
                    &cpu.memory().mar().write().control_info()})}));

  BusValidator validator;
  EXPECT_THROW(validator.Run(set), MicrocodeError);
}

TEST(BusValidatorTest, RejectsReaderWithoutWriter) {
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP,
      {MakeStep(0, {&cpu.a().read().control_info()})}));

  BusValidator validator;
  EXPECT_THROW(validator.Run(set), MicrocodeError);
}

TEST(BusValidatorTest, AcceptsControlsNotUsingBuses) {
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP,
      {MakeStep(0, {&cpu.halt().control_info()})}));

  BusValidator validator;
  EXPECT_NO_THROW(validator.Run(set));
}

TEST(BusValidatorTest, ValidatesFetchPreamble) {
  Cpu cpu;
  InstructionSet set;
  // Invalid: reader without writer in fetch preamble
  set.fetch_preamble.push_back(MakeStep(0, {&cpu.a().read().control_info()}));

  BusValidator validator;
  EXPECT_THROW(validator.Run(set), MicrocodeError);
}

TEST(BusValidatorTest, AcceptsMemoryDataReadWrite) {
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP,
      {MakeStep(0, {&cpu.memory().write().control_info(),
                    &cpu.a().read().control_info()})}));

  BusValidator validator;
  EXPECT_NO_THROW(validator.Run(set));
}
