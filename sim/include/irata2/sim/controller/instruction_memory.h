#ifndef IRATA2_SIM_CONTROLLER_INSTRUCTION_MEMORY_H
#define IRATA2_SIM_CONTROLLER_INSTRUCTION_MEMORY_H

#include <cstdint>
#include <memory>
#include <vector>

#include "irata2/microcode/output/program.h"
#include "irata2/sim/component.h"
#include "irata2/sim/control.h"
#include "irata2/sim/controller/control_encoder.h"
#include "irata2/sim/controller/status_encoder.h"
#include "irata2/sim/rom_storage.h"

namespace irata2::sim {
class Cpu;
}

namespace irata2::sim::controller {

/// Type alias for microcode ROM storage (32-bit address, 64-bit data)
using MicrocodeRomStorage = RomStorage<uint32_t, uint64_t>;

/// Hardware-ish ROM storage for microcode.
///
/// InstructionMemory encapsulates the microcode lookup functionality,
/// containing ControlEncoder and StatusEncoder for bidirectional mapping.
/// At construction, it processes the microcode program and "burns" it into
/// ROM storage. The original program is not retained after initialization.
///
/// Uses RomStorage<uint32_t, uint64_t>:
/// - 32-bit addresses encode (opcode << 16 | step << 8 | status)
/// - 64-bit data stores control words
class InstructionMemory final : public ComponentWithParent {
 public:
  /// Construct InstructionMemory from a microcode program.
  ///
  /// Initializes the encoders and builds the internal lookup table.
  /// The microcode program is not retained after initialization.
  ///
  /// \param name Component name
  /// \param parent Parent component
  /// \param program The microcode program to encode
  /// \param cpu The CPU containing control and status references
  InstructionMemory(std::string name,
                    Component& parent,
                    const microcode::output::MicrocodeProgram& program,
                    Cpu& cpu);

  /// Look up control signals for a given instruction state.
  ///
  /// Returns the set of controls that should be asserted for the given
  /// opcode, step, and status combination.
  ///
  /// \param opcode The instruction opcode
  /// \param step The microcode step within the instruction
  /// \param status The current status register value
  /// \return Vector of control references to assert
  /// \throws SimError if the lookup fails (missing microcode entry)
  std::vector<ControlBase*> Lookup(uint8_t opcode,
                                    uint8_t step,
                                    uint8_t status) const;

  /// Get the control encoder.
  const ControlEncoder& control_encoder() const { return control_encoder_; }

  /// Get the status encoder.
  const StatusEncoder& status_encoder() const { return status_encoder_; }

  /// Get the ROM storage (for debugging/inspection).
  const MicrocodeRomStorage* rom() const { return rom_.get(); }

 private:
  ControlEncoder control_encoder_;
  StatusEncoder status_encoder_;
  std::unique_ptr<MicrocodeRomStorage> rom_;
};

}  // namespace irata2::sim::controller

#endif  // IRATA2_SIM_CONTROLLER_INSTRUCTION_MEMORY_H
