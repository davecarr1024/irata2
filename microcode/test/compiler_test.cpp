#include "irata2/microcode/compiler/compiler.h"

#include "irata2/hdl.h"
#include "irata2/microcode/encoder/control_encoder.h"
#include "irata2/microcode/encoder/status_encoder.h"
#include "irata2/microcode/error.h"

#include <algorithm>
#include <gtest/gtest.h>

using irata2::hdl::Cpu;
using irata2::microcode::compiler::Compiler;
using irata2::microcode::encoder::ControlEncoder;
using irata2::microcode::encoder::StatusEncoder;
using irata2::microcode::ir::Instruction;
using irata2::microcode::ir::InstructionSet;
using irata2::microcode::ir::InstructionVariant;
using irata2::microcode::ir::Step;
using irata2::microcode::output::EncodeKey;
using irata2::microcode::output::MicrocodeKey;
using irata2::isa::Opcode;
using irata2::microcode::MicrocodeError;

namespace {
Instruction MakeInstruction(Opcode opcode, std::vector<Step> steps) {
  Instruction instruction;
  instruction.opcode = opcode;
  InstructionVariant variant;
  variant.steps = std::move(steps);
  instruction.variants.push_back(std::move(variant));
  return instruction;
}

Step MakeStep(std::initializer_list<const irata2::hdl::ControlBase*> controls) {
  Step step;
  step.controls = {controls.begin(), controls.end()};
  return step;
}
}  // namespace

TEST(CompilerTest, ProducesMicrocodeTable) {
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(Opcode::HLT_IMP,
                                             {MakeStep({&cpu.halt()})}));
  set.instructions.push_back(MakeInstruction(Opcode::NOP_IMP,
                                             {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::CRS_IMP,
                                             {MakeStep({&cpu.crash()})}));

  ControlEncoder control_encoder(cpu);
  StatusEncoder status_encoder({});

  Compiler compiler(control_encoder,
                    status_encoder,
                    cpu.controller().sc().increment(),
                    cpu.controller().sc().reset());
  const auto program = compiler.Compile(set);

  MicrocodeKey key;
  key.opcode = static_cast<uint8_t>(Opcode::HLT_IMP);
  key.step = 0;
  key.status = 0;
  const auto it = program.table.find(EncodeKey(key));
  ASSERT_NE(it, program.table.end());

  const auto decoded = control_encoder.Decode(it->second);
  EXPECT_NE(std::find(decoded.begin(), decoded.end(), "halt"), decoded.end());
  EXPECT_NE(std::find(decoded.begin(), decoded.end(),
                      "controller.sc.reset"), decoded.end());
}

TEST(CompilerTest, RejectsStepIndexOverflow) {
  Cpu cpu;
  InstructionSet set;
  std::vector<Step> steps(257);
  set.instructions.push_back(MakeInstruction(Opcode::HLT_IMP, std::move(steps)));
  set.instructions.push_back(MakeInstruction(Opcode::NOP_IMP, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::CRS_IMP, {MakeStep({})}));

  ControlEncoder control_encoder(cpu);
  StatusEncoder status_encoder({});

  Compiler compiler(control_encoder,
                    status_encoder,
                    cpu.controller().sc().increment(),
                    cpu.controller().sc().reset());
  EXPECT_THROW(compiler.Compile(set), MicrocodeError);
}
