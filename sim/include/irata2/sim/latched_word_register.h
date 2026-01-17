#ifndef IRATA2_SIM_LATCHED_WORD_REGISTER_H
#define IRATA2_SIM_LATCHED_WORD_REGISTER_H

#include <utility>

#include "irata2/base/types.h"
#include "irata2/sim/bus.h"
#include "irata2/sim/component.h"
#include "irata2/sim/control.h"

namespace irata2::sim {

/// Latched word register that captures values from a bus.
///
/// This register caches a word value from a bus. When the latch control is
/// asserted during the Process phase, it copies the bus's current value.
/// This provides a hardware-style "instruction pointer cache" pattern where
/// IPC captures PC's value from the address bus before PC increments.
///
/// Note: This reads from a bus, not directly from a register, to preserve
/// proper timing semantics (captures the value written to the bus in the
/// Write phase, even if the source register increments in the Process phase).
class LatchedWordRegister final : public ComponentWithParent {
 public:
  LatchedWordRegister(std::string name, Component& parent, Bus<base::Word>& bus)
      : ComponentWithParent(parent, std::move(name)),
        latch_control_("latch", *this),
        bus_(bus) {}

  const base::Word& value() const { return value_; }
  void set_value(base::Word value) { value_ = value; }

  ProcessControl<true>& latch() { return latch_control_; }
  const ProcessControl<true>& latch() const { return latch_control_; }

  void TickProcess() override {
    Component::TickProcess();

    if (latch_control_.asserted()) {
      value_ = bus_.value();
    }
  }

 private:
  ProcessControl<true> latch_control_;
  Bus<base::Word>& bus_;
  base::Word value_{};
};

}  // namespace irata2::sim

#endif  // IRATA2_SIM_LATCHED_WORD_REGISTER_H
