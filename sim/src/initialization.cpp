#include "irata2/sim/initialization.h"

#include "irata2/microcode/compiler/compiler.h"
#include "irata2/microcode/ir/builder.h"

namespace irata2::sim {

namespace {
using irata2::microcode::ir::Instruction;
using irata2::microcode::ir::InstructionSet;
using irata2::microcode::ir::InstructionVariant;
using irata2::microcode::ir::Step;
using irata2::microcode::output::MicrocodeProgram;
using irata2::microcode::output::StatusBitDefinition;

std::vector<StatusBitDefinition> BuildStatusBits(const hdl::StatusRegister& status) {
  return {
      {status.negative().name(), static_cast<uint8_t>(status.negative().bit_index())},
      {status.overflow().name(), static_cast<uint8_t>(status.overflow().bit_index())},
      {status.unused().name(), static_cast<uint8_t>(status.unused().bit_index())},
      {status.brk().name(), static_cast<uint8_t>(status.brk().bit_index())},
      {status.decimal().name(), static_cast<uint8_t>(status.decimal().bit_index())},
      {status.interrupt_disable().name(),
       static_cast<uint8_t>(status.interrupt_disable().bit_index())},
      {status.zero().name(), static_cast<uint8_t>(status.zero().bit_index())},
      {status.carry().name(), static_cast<uint8_t>(status.carry().bit_index())},
  };
}

Instruction MakeInstruction(isa::Opcode opcode,
                            std::vector<const hdl::ControlBase*> controls) {
  Instruction instruction;
  instruction.opcode = opcode;
  InstructionVariant variant;
  Step step;
  step.controls = std::move(controls);
  variant.steps.push_back(std::move(step));
  instruction.variants.push_back(std::move(variant));
  return instruction;
}

MicrocodeProgram BuildMicrocodeProgram(const hdl::Cpu& hdl) {
  microcode::ir::Builder builder(hdl);
  InstructionSet instruction_set;
  instruction_set.fetch_preamble = {};
  instruction_set.instructions = {
      MakeInstruction(isa::Opcode::HLT_IMP,
                      {builder.RequireControl("halt", "HLT")}),
      MakeInstruction(isa::Opcode::NOP_IMP, {}),
      MakeInstruction(isa::Opcode::CRS_IMP,
                      {builder.RequireControl("crash", "CRS")}),
  };

  microcode::encoder::ControlEncoder control_encoder(hdl);
  microcode::encoder::StatusEncoder status_encoder(BuildStatusBits(hdl.status()));
  microcode::compiler::Compiler compiler(control_encoder,
                                         status_encoder,
                                         hdl.controller().sc().increment(),
                                         hdl.controller().sc().reset());
  return compiler.Compile(std::move(instruction_set));
}
}  // namespace

std::shared_ptr<const hdl::Cpu> DefaultHdl() {
  static const auto hdl = std::make_shared<const hdl::Cpu>();
  return hdl;
}

std::shared_ptr<const microcode::output::MicrocodeProgram> DefaultMicrocodeProgram() {
  static const auto program = []() {
    auto built = BuildMicrocodeProgram(*DefaultHdl());
    return std::make_shared<const microcode::output::MicrocodeProgram>(std::move(built));
  }();
  return program;
}

}  // namespace irata2::sim
