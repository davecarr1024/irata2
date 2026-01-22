#include "irata2/sim/initialization.h"

#include "irata2/microcode/compiler/compiler.h"
#include "irata2/microcode/ir/irata_instruction_set.h"

namespace irata2::sim {

namespace {
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

MicrocodeProgram BuildMicrocodeProgram(const hdl::Cpu& hdl) {
  microcode::ir::InstructionSet instruction_set =
      microcode::ir::BuildIrataInstructionSet(hdl);

  microcode::encoder::ControlEncoder control_encoder(hdl);
  microcode::encoder::StatusEncoder status_encoder(BuildStatusBits(hdl.status()));
  microcode::compiler::Compiler compiler(control_encoder,
                                         status_encoder,
                                         hdl,
                                         hdl.controller().sc().increment().control_info(),
                                         hdl.controller().sc().reset().control_info());
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
