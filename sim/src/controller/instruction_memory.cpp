#include "irata2/sim/controller/instruction_memory.h"

#include "irata2/sim/cpu.h"
#include "irata2/sim/error.h"

#include <algorithm>
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

  // Find the maximum address to size the ROM
  // If table is empty, create minimal ROM
  uint32_t max_address = 0;
  for (const auto& [key, value] : program.table) {
    max_address = std::max(max_address, key);
  }

  // Create ROM sized to fit all entries (add 1 because addresses are 0-indexed)
  const size_t rom_size = static_cast<size_t>(max_address) + 1;
  std::vector<uint64_t> rom_data(rom_size, 0);

  // Burn the microcode into ROM
  for (const auto& [key, value] : program.table) {
    if (key >= rom_data.size()) {
      std::ostringstream message;
      message << "microcode key " << key << " exceeds ROM size "
              << rom_data.size();
      throw SimError(message.str());
    }
    rom_data[key] = value;
  }

  // Create the ROM with the burned microcode
  rom_ = std::make_unique<MicrocodeRomStorage>("rom", *this, std::move(rom_data));
}

std::vector<ControlBase*> InstructionMemory::Lookup(uint8_t opcode,
                                                      uint8_t step,
                                                      uint8_t status) const {
  // Build ROM address from (opcode, step, status)
  const uint32_t address = (static_cast<uint32_t>(opcode) << 16) |
                           (static_cast<uint32_t>(step) << 8) |
                           static_cast<uint32_t>(status);

  // Read control word from ROM
  const uint64_t control_word = rom_->Read(address);

  // Decode control word to get control references
  // Note: control_word == 0 is valid (no controls asserted)
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
