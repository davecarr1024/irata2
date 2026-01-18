#include "irata2/microcode/compiler/compiler.h"

#include "irata2/hdl.h"
#include "irata2/microcode/encoder/control_encoder.h"
#include "irata2/microcode/encoder/status_encoder.h"
#include "irata2/microcode/error.h"

#include <algorithm>
#include <gtest/gtest.h>

using irata2::hdl::ControlInfo;
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

Step MakeStep(std::initializer_list<const ControlInfo*> controls) {
  Step step;
  step.controls = {controls.begin(), controls.end()};
  return step;
}
}  // namespace

TEST(CompilerTest, ProducesMicrocodeTable) {
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      Opcode::HLT_IMP, {MakeStep({&cpu.halt().control_info()})}));
  set.instructions.push_back(MakeInstruction(Opcode::NOP_IMP, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(
      Opcode::CRS_IMP, {MakeStep({&cpu.crash().control_info()})}));
  set.instructions.push_back(MakeInstruction(Opcode::LDA_IMM, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::CMP_IMM, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::JEQ_ABS, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::BEQ_REL, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::BNE_REL, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::BCS_REL, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::BCC_REL, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::BMI_REL, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::BPL_REL, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::BVS_REL, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::BVC_REL, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::ADC_IMM, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::SBC_IMM, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::AND_IMM, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::ORA_IMM, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::EOR_IMM, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::ASL_IMP, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::LSR_IMP, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::ROL_IMP, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::ROR_IMP, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::TAX_IMP, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::TXA_IMP, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::LDX_IMM, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::LDA_ZP, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::STA_ZP, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::LDX_ZP, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::STX_ZP, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::ADC_ZP, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::SBC_ZP, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::AND_ZP, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::ORA_ZP, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::EOR_ZP, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::CMP_ZP, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::ASL_ZP, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::LSR_ZP, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::ROL_ZP, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::ROR_ZP, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::LDA_ABS, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::STA_ABS, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::LDX_ABS, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::STX_ABS, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::ADC_ABS, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::SBC_ABS, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::AND_ABS, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::ORA_ABS, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::EOR_ABS, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::CMP_ABS, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::ASL_ABS, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::LSR_ABS, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::ROL_ABS, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::ROR_ABS, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::INX_IMP, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::DEX_IMP, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::INC_ZP, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::DEC_ZP, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::INC_ABS, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::DEC_ABS, {MakeStep({})}));

  ControlEncoder control_encoder(cpu);
  StatusEncoder status_encoder({});

  Compiler compiler(control_encoder, status_encoder,
                    cpu.controller().sc().increment().control_info(),
                    cpu.controller().sc().reset().control_info());
  const auto program = compiler.Compile(set);

  MicrocodeKey key;
  key.opcode = static_cast<uint8_t>(Opcode::HLT_IMP);
  key.step = 0;
  key.status = 0;
  const auto it = program.table.find(EncodeKey(key));
  ASSERT_NE(it, program.table.end());

  const auto decoded = control_encoder.Decode(it->second);
  EXPECT_NE(std::find(decoded.begin(), decoded.end(), "halt"), decoded.end());
  EXPECT_NE(std::find(decoded.begin(), decoded.end(), "controller.sc.reset"),
            decoded.end());
}

TEST(CompilerTest, RejectsStepIndexOverflow) {
  Cpu cpu;
  InstructionSet set;
  // Create 257 non-empty, non-duplicate steps so optimizers don't remove them
  std::vector<Step> steps;
  steps.reserve(257);
  for (int i = 0; i < 257; ++i) {
    // Alternate between halt and crash to prevent deduplication
    if (i % 2 == 0) {
      steps.push_back(MakeStep({&cpu.halt().control_info()}));
    } else {
      steps.push_back(MakeStep({&cpu.crash().control_info()}));
    }
  }
  set.instructions.push_back(
      MakeInstruction(Opcode::HLT_IMP, std::move(steps)));
  set.instructions.push_back(MakeInstruction(Opcode::NOP_IMP, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::CRS_IMP, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::LDA_IMM, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::CMP_IMM, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::JEQ_ABS, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::BEQ_REL, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::BNE_REL, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::BCS_REL, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::BCC_REL, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::BMI_REL, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::BPL_REL, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::BVS_REL, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::BVC_REL, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::ADC_IMM, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::SBC_IMM, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::AND_IMM, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::ORA_IMM, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::EOR_IMM, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::ASL_IMP, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::LSR_IMP, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::ROL_IMP, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::ROR_IMP, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::TAX_IMP, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::TXA_IMP, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::LDX_IMM, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::LDA_ZP, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::STA_ZP, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::LDX_ZP, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::STX_ZP, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::ADC_ZP, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::SBC_ZP, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::AND_ZP, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::ORA_ZP, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::EOR_ZP, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::CMP_ZP, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::ASL_ZP, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::LSR_ZP, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::ROL_ZP, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::ROR_ZP, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::LDA_ABS, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::STA_ABS, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::LDX_ABS, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::STX_ABS, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::ADC_ABS, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::SBC_ABS, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::AND_ABS, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::ORA_ABS, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::EOR_ABS, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::CMP_ABS, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::ASL_ABS, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::LSR_ABS, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::ROL_ABS, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::ROR_ABS, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::INX_IMP, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::DEX_IMP, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::INC_ZP, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::DEC_ZP, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::INC_ABS, {MakeStep({})}));
  set.instructions.push_back(MakeInstruction(Opcode::DEC_ABS, {MakeStep({})}));

  ControlEncoder control_encoder(cpu);
  StatusEncoder status_encoder({});

  Compiler compiler(control_encoder, status_encoder,
                    cpu.controller().sc().increment().control_info(),
                    cpu.controller().sc().reset().control_info());
  EXPECT_THROW(compiler.Compile(set), MicrocodeError);
}

TEST(CompilerTest, RejectsOpcodeOutOfRange) {
  Cpu cpu;
  InstructionSet set;
  set.instructions.push_back(MakeInstruction(
      static_cast<Opcode>(0x1FF), {MakeStep({})}));

  ControlEncoder control_encoder(cpu);
  StatusEncoder status_encoder({});

  Compiler compiler(control_encoder, status_encoder,
                    cpu.controller().sc().increment().control_info(),
                    cpu.controller().sc().reset().control_info());
  EXPECT_THROW(compiler.Compile(set), MicrocodeError);
}
