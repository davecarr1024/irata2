#include "irata2/microcode/compiler/compiler.h"

#include "irata2/hdl/cpu.h"
#include "irata2/microcode/encoder/control_encoder.h"
#include "irata2/microcode/encoder/status_encoder.h"
#include "irata2/microcode/error.h"

#include <gtest/gtest.h>

using irata2::hdl::Cpu;
using irata2::microcode::MicrocodeError;
using irata2::microcode::compiler::Compiler;
using irata2::microcode::encoder::ControlEncoder;
using irata2::microcode::encoder::StatusEncoder;
using irata2::microcode::ir::Instruction;
using irata2::microcode::ir::InstructionSet;
using irata2::microcode::ir::InstructionVariant;
using irata2::microcode::ir::Step;
using irata2::isa::Opcode;

namespace {
Instruction MakeInstruction(Opcode opcode,
                            std::vector<InstructionVariant> variants) {
  Instruction instruction;
  instruction.opcode = opcode;
  instruction.variants = std::move(variants);
  return instruction;
}

InstructionVariant MakeVariant(std::initializer_list<const irata2::hdl::ControlBase*> controls) {
  InstructionVariant variant;
  Step step;
  step.controls = {controls.begin(), controls.end()};
  variant.steps.push_back(step);
  return variant;
}
}  // namespace

TEST(CompilerConflictTest, RejectsConflictingMicrocode) {
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP,
      {MakeVariant({&cpu.halt()}), MakeVariant({&cpu.crash()})}));
  set.instructions.push_back(MakeInstruction(Opcode::NOP_IMP, {MakeVariant({})}));
  set.instructions.push_back(MakeInstruction(Opcode::CRS_IMP, {MakeVariant({})}));

  ControlEncoder control_encoder(cpu);
  StatusEncoder status_encoder({});

  Compiler compiler(control_encoder,
                    status_encoder,
                    cpu.controller().sc().increment(),
                    cpu.controller().sc().reset());

  EXPECT_THROW(compiler.Compile(set), MicrocodeError);
}
