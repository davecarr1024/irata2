#include "irata2/sim/controller/controller.h"

#include "irata2/sim/cpu.h"
#include "irata2/sim/error.h"

#include <sstream>

namespace irata2::sim::controller {

Controller::Controller(std::string name,
                       Component& parent,
                       Bus<base::Byte>& data_bus,
                       const ProgramCounter& pc)
    : ComponentWithParent(parent, std::move(name)),
      instruction_start_("instruction_start", *this),
      ir_("ir",
          *this,
          data_bus,
          cpu().irq_line(),
          instruction_start_,
          cpu().status().interrupt_disable()),
      sc_("sc", *this),
      pc_(pc),
      ipc_("ipc", *this, pc),
      instruction_memory_(nullptr) {}

void Controller::LoadProgram(
    std::shared_ptr<const microcode::output::MicrocodeProgram> program) {
  if (!program) {
    throw SimError("microcode program is null");
  }

  // Create InstructionMemory with the program
  instruction_memory_ = std::make_unique<InstructionMemory>(
      "instruction_memory", *this, *program, cpu());
}

void Controller::TickControl() {
  if (!instruction_memory_) {
    throw SimError("controller has no microcode program");
  }

  const uint8_t opcode = ir_.value().value();
  const uint8_t step = sc_.value().value();
  const uint8_t status = instruction_memory_->status_encoder().Encode();

  const auto controls = instruction_memory_->Lookup(opcode, step, status);
  for (auto* control : controls) {
    control->Assert();
  }
}

void Controller::TickProcess() {
  // IPC latching occurs at instruction start to capture the pre-increment PC.
  Component::TickProcess();

  if (instruction_start_.asserted()) {
    ipc_.set_value(pc_.value());
  }
}

}  // namespace irata2::sim::controller
