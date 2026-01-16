#ifndef IRATA2_SIM_CONTROLLER_INSTRUCTION_MEMORY_H
#define IRATA2_SIM_CONTROLLER_INSTRUCTION_MEMORY_H

#include <cstdint>
#include <memory>
#include <unordered_map>
#include <vector>

#include "irata2/microcode/output/program.h"
#include "irata2/sim/component.h"
#include "irata2/sim/control.h"
#include "irata2/sim/controller/control_encoder.h"
#include "irata2/sim/controller/status_encoder.h"

namespace irata2::sim {
class Cpu;
}

namespace irata2::sim::controller {

/// Hardware-ish ROM storage for microcode.
///
/// InstructionMemory encapsulates the microcode lookup functionality,
/// containing ControlEncoder and StatusEncoder for bidirectional mapping.
/// At construction, it processes the microcode program and builds an
/// internal lookup table. The original program is not retained after
/// initialization.
///
/// This implementation uses a lookup table rather than a full ROM grid.
/// A future enhancement could implement actual ROM chips for a more
/// hardware-accurate simulation.
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

 private:
  ControlEncoder control_encoder_;
  StatusEncoder status_encoder_;

  // Lookup table: (opcode << 16) | (step << 8) | status -> control_word
  std::unordered_map<uint32_t, uint64_t> table_;
};

}  // namespace irata2::sim::controller

#endif  // IRATA2_SIM_CONTROLLER_INSTRUCTION_MEMORY_H
