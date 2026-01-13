#include "irata2/sim/controller/controller.h"

#include "irata2/microcode/encoder/instruction_encoder.h"
#include "irata2/sim/cpu.h"
#include "irata2/sim/error.h"

#include <sstream>

namespace irata2::sim::controller {

Controller::Controller(std::string name,
                       Component& parent,
                       Bus<base::Byte>& data_bus,
                       Bus<base::Word>& address_bus)
    : ComponentWithParent(parent, std::move(name)),
      ir_("ir", *this, data_bus),
      sc_("sc", *this),
      ipc_("ipc", *this),
      ipc_latch_("ipc_latch", *this),
      address_bus_(address_bus) {}

void Controller::LoadProgram(
    std::shared_ptr<const microcode::output::MicrocodeProgram> program) {
  if (!program) {
    throw SimError("microcode program is null");
  }
  if (program->control_paths.size() > 64) {
    std::ostringstream message;
    message << "too many controls for instruction memory: "
            << program->control_paths.size();
    throw SimError(message.str());
  }

  for (const auto& bit : program->status_bits) {
    if (bit.bit >= 8) {
      std::ostringstream message;
      message << "status bit out of range: " << static_cast<int>(bit.bit);
      throw SimError(message.str());
    }
  }

  program_ = std::move(program);
  control_lines_.clear();
  control_lines_.reserve(program_->control_paths.size());

  const auto& control_order = cpu().ControlOrder();
  if (control_order.size() != program_->control_paths.size()) {
    throw SimError("control table size mismatch between HDL and microcode");
  }

  for (size_t i = 0; i < control_order.size(); ++i) {
    auto* control = control_order[i];
    const auto& expected = program_->control_paths[i];
    if (control->path() != expected) {
      throw SimError("control path order mismatch: " + expected);
    }
    control_lines_.push_back(control);
  }
}

uint8_t Controller::EncodeStatus() const {
  if (!program_) {
    throw SimError("controller has no microcode program");
  }
  if (program_->status_bits.empty()) {
    return 0;
  }
  return cpu().status().value().value();
}

uint64_t Controller::LookupControlWord(uint8_t opcode,
                                       uint8_t step,
                                       uint8_t status) const {
  if (!program_) {
    throw SimError("controller has no microcode program");
  }
  const auto key = microcode::encoder::InstructionEncoder::MakeKey(
      static_cast<isa::Opcode>(opcode), step, status);
  const uint32_t encoded_key = microcode::output::EncodeKey(key);
  const auto it = program_->table.find(encoded_key);
  if (it == program_->table.end()) {
    std::ostringstream message;
    message << "microcode missing for opcode "
            << static_cast<int>(opcode)
            << " step " << static_cast<int>(step)
            << " status " << static_cast<int>(status);
    throw SimError(message.str());
  }
  return it->second;
}

void Controller::AssertControlWord(uint64_t control_word) {
  if (control_lines_.size() < 64) {
    const uint64_t overflow = control_word >> control_lines_.size();
    if (overflow != 0) {
      throw SimError("control word sets bits outside control table");
    }
  }

  for (size_t i = 0; i < control_lines_.size(); ++i) {
    if ((control_word >> i) & 1U) {
      control_lines_[i]->Assert();
    }
  }
}

void Controller::TickControl() {
  if (!program_) {
    throw SimError("controller has no microcode program");
  }

  const uint8_t opcode = ir_.value().value();
  const uint8_t step = sc_.value().value();
  const uint8_t status = EncodeStatus();
  const uint64_t control_word = LookupControlWord(opcode, step, status);
  AssertControlWord(control_word);
}

void Controller::TickProcess() {
  if (ipc_latch_.asserted()) {
    ipc_.set_value(address_bus_.value());
  }
}

}  // namespace irata2::sim::controller
