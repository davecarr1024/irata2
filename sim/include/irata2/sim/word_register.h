#ifndef IRATA2_SIM_WORD_REGISTER_H
#define IRATA2_SIM_WORD_REGISTER_H

#include <utility>

#include "irata2/base/types.h"
#include "irata2/sim/register_with_bus.h"

namespace irata2::sim {

/// Word register with byte-level accessors.
///
/// This register stores a 16-bit word and provides access to its high and low
/// bytes. The register is connected to a word bus for full word operations.
class WordRegister : public RegisterWithBus<WordRegister, base::Word> {
 public:
  WordRegister(std::string name, Component& parent, Bus<base::Word>& bus)
      : RegisterWithBus<WordRegister, base::Word>(std::move(name), parent, bus) {}

  /// Get the low byte of the word value.
  base::Byte low() const { return value().low(); }

  /// Get the high byte of the word value.
  base::Byte high() const { return value().high(); }

  /// Set the low byte while preserving the high byte.
  void set_low(base::Byte byte) {
    set_value(base::Word(high(), byte));
  }

  /// Set the high byte while preserving the low byte.
  void set_high(base::Byte byte) {
    set_value(base::Word(byte, low()));
  }
};

}  // namespace irata2::sim

#endif  // IRATA2_SIM_WORD_REGISTER_H
