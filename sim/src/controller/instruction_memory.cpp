#include "irata2/sim/controller/instruction_memory.h"

#include "irata2/microcode/encoder/instruction_encoder.h"
#include "irata2/sim/cpu.h"
#include "irata2/sim/error.h"

#include <sstream>

namespace irata2::sim::controller {

InstructionMemory::InstructionMemory(
    std::string name,
    Component& parent,
    const microcode::output::MicrocodeProgram& program,
    Cpu& cpu)
    : ComponentWithParent(parent, std::move(name)),
      control_encoder_("control_encoder", *this),
      status_encoder_("status_encoder", *this) {
  // Initialize encoders
  control_encoder_.Initialize(program, cpu);
  status_encoder_.Initialize(program, cpu);

  // Copy the lookup table
  table_ = program.table;
}

std::vector<ControlBase*> InstructionMemory::Lookup(uint8_t opcode,
                                                      uint8_t step,
                                                      uint8_t status) const {
  // Build lookup key
  const uint32_t key = (static_cast<uint32_t>(opcode) << 16) |
                       (static_cast<uint32_t>(step) << 8) |
                       static_cast<uint32_t>(status);

  // Lookup in table
  const auto it = table_.find(key);
  if (it == table_.end()) {
    std::ostringstream message;
    message << "microcode missing for opcode " << static_cast<int>(opcode)
            << " step " << static_cast<int>(step) << " status "
            << static_cast<int>(status);
    throw SimError(message.str());
  }

  const uint64_t control_word = it->second;

  // Decode control word to get control references
  std::vector<ControlBase*> controls;
  const size_t num_controls = control_encoder_.control_count();

  // Check for overflow
  if (num_controls < 64) {
    const uint64_t overflow = control_word >> num_controls;
    if (overflow != 0) {
      throw SimError("control word sets bits outside control table");
    }
  }

  // Extract controls from control word
  for (size_t i = 0; i < num_controls; ++i) {
    if ((control_word >> i) & 1U) {
      controls.push_back(control_encoder_.GetControl(i));
    }
  }

  return controls;
}

}  // namespace irata2::sim::controller
