#ifndef IRATA2_SIM_CONTROLLER_CONTROL_ENCODER_H
#define IRATA2_SIM_CONTROLLER_CONTROL_ENCODER_H

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#include "irata2/microcode/output/program.h"
#include "irata2/sim/component.h"
#include "irata2/sim/control.h"

namespace irata2::sim {
class Cpu;
}

namespace irata2::sim::controller {

/// Encodes and decodes control signals to/from binary control words.
///
/// The ControlEncoder maintains an ordered list of control references and
/// provides bidirectional mapping between control signals and their binary
/// representation. Each control is assigned a bit position based on its index
/// in the ordered list.
class ControlEncoder final : public ComponentWithParent {
 public:
  ControlEncoder(std::string name, Component& parent);

  /// Initialize the encoder from a microcode program.
  ///
  /// Extracts control paths from the program, fetches corresponding control
  /// references from the CPU, and validates that the ordering matches.
  ///
  /// \param program The microcode program containing control path definitions
  /// \param cpu The CPU containing the actual control references
  /// \throws SimError if validation fails
  void Initialize(const microcode::output::MicrocodeProgram& program,
                  Cpu& cpu);

  /// Encode a set of controls into a binary control word.
  ///
  /// Each asserted control sets the corresponding bit in the output word.
  ///
  /// \param controls The controls to encode
  /// \return Binary control word as a vector of bytes (little-endian)
  std::vector<uint8_t> Encode(
      const std::vector<ControlBase*>& controls) const;

  /// Decode a binary control word into a set of control references.
  ///
  /// Each set bit in the control word is mapped to its corresponding control.
  ///
  /// \param word Binary control word as a vector of bytes (little-endian)
  /// \return The control references corresponding to set bits
  std::vector<ControlBase*> Decode(const std::vector<uint8_t>& word) const;

  /// Get the width of the control word in bytes.
  size_t control_word_width() const { return control_word_width_; }

  /// Get the number of controls.
  size_t control_count() const { return control_references_.size(); }

  /// Get a control reference by index.
  ///
  /// \param index The control index (0 to control_count()-1)
  /// \return The control reference at the given index
  /// \throws SimError if index is out of range
  ControlBase* GetControl(size_t index) const;

 private:
  std::vector<ControlBase*> control_references_;  // Ordered list
  size_t control_word_width_;  // Bytes needed to encode all controls
};

}  // namespace irata2::sim::controller

#endif  // IRATA2_SIM_CONTROLLER_CONTROL_ENCODER_H
