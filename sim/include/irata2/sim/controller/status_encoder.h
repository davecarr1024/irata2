#ifndef IRATA2_SIM_CONTROLLER_STATUS_ENCODER_H
#define IRATA2_SIM_CONTROLLER_STATUS_ENCODER_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "irata2/microcode/output/program.h"
#include "irata2/sim/component.h"
#include "irata2/sim/status.h"

namespace irata2::sim {
class Cpu;
}

namespace irata2::sim::controller {

/// Encodes and decodes status values for ROM addressing.
///
/// The StatusEncoder maintains an ordered list of status bit references and
/// provides encoding for ROM addressing. It supports expansion of partial
/// status requirements (where some bits are "don't care") into all matching
/// complete status values for ROM population.
class StatusEncoder final : public ComponentWithParent {
 public:
  StatusEncoder(std::string name, Component& parent);

  /// Initialize the encoder from a microcode program.
  ///
  /// Extracts status bit definitions from the program and fetches corresponding
  /// status references from the CPU.
  ///
  /// \param program The microcode program containing status bit definitions
  /// \param cpu The CPU containing the actual status references
  /// \throws SimError if validation fails
  void Initialize(const microcode::output::MicrocodeProgram& program,
                  Cpu& cpu);

  /// Encode current status values into a binary encoding for ROM addressing.
  ///
  /// Reads the current values of all status bits and packs them into a
  /// compact encoding suitable for ROM address generation.
  ///
  /// \return Binary encoding of status bits
  uint8_t Encode() const;

  /// Decode a binary encoding into status bit values.
  ///
  /// Converts a packed status encoding back into individual bit values.
  ///
  /// \param encoded Binary encoding of status bits
  /// \return Vector of bit values in the same order as status_references_
  std::vector<bool> Decode(uint8_t encoded) const;

  /// Expand a partial status specification to all matching complete values.
  ///
  /// When a microcode step only cares about some status bits (e.g., only
  /// the carry flag), this generates all possible values for the "don't care"
  /// bits, allowing the ROM to be fully populated.
  ///
  /// \param status_mask Bitmask indicating which status bits are specified
  /// \param status_value Values for the specified bits
  /// \return All matching complete status encodings
  std::vector<uint8_t> Permute(uint8_t status_mask,
                                uint8_t status_value) const;

  /// Get the number of status bits being encoded.
  size_t status_bit_count() const { return status_references_.size(); }

 private:
  std::vector<Status*> status_references_;  // Ordered list for stable encoding
};

}  // namespace irata2::sim::controller

#endif  // IRATA2_SIM_CONTROLLER_STATUS_ENCODER_H
