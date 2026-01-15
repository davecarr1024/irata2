#ifndef IRATA2_SIM_WORD_COUNTER_H
#define IRATA2_SIM_WORD_COUNTER_H

#include <utility>

#include "irata2/sim/control.h"
#include "irata2/sim/word_register.h"

namespace irata2::sim {

/// Word counter with proper byte overflow handling.
///
/// This register stores a 16-bit word value and can increment it by 1.
/// When the low byte overflows (0xFF -> 0x00), the high byte is automatically
/// incremented, implementing standard 16-bit increment behavior.
class WordCounter : public WordRegister {
 public:
  WordCounter(std::string name, Component& parent, Bus<base::Word>& bus)
      : WordRegister(std::move(name), parent, bus),
        increment_control_("increment", *this) {}

  ProcessControl<true>& increment() { return increment_control_; }
  const ProcessControl<true>& increment() const { return increment_control_; }

  void TickProcess() override {
    // First handle reset and children (from RegisterWithBus via WordRegister)
    WordRegister::TickProcess();

    // Then handle increment (after reset, so reset takes priority)
    if (increment_control_.asserted() && !reset().asserted()) {
      // Increment the full word value
      set_value(value() + base::Word{1});
    }
  }

 private:
  ProcessControl<true> increment_control_;
};

}  // namespace irata2::sim

#endif  // IRATA2_SIM_WORD_COUNTER_H
