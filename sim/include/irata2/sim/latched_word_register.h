#ifndef IRATA2_SIM_LATCHED_WORD_REGISTER_H
#define IRATA2_SIM_LATCHED_WORD_REGISTER_H

#include <utility>

#include "irata2/base/types.h"
#include "irata2/sim/component.h"
#include "irata2/sim/control.h"
#include "irata2/sim/program_counter.h"

namespace irata2::sim {

/// Latched word register that captures values from the program counter.
///
/// When the latch control is asserted during the Process phase, it copies
/// the PC's current value. This provides a hardware-style "instruction
/// pointer cache" pattern where IPC captures PC's value before PC increments
/// (both happen in Process phase, but latch runs first).
class LatchedWordRegister final : public ComponentWithParent {
 public:
  LatchedWordRegister(std::string name,
                      Component& parent,
                      const ProgramCounter& source)
      : ComponentWithParent(parent, std::move(name)),
        latch_control_("latch", *this),
        source_(source) {}

  const base::Word& value() const { return value_; }
  void set_value(base::Word value) { value_ = value; }

  ProcessControl<true>& latch() { return latch_control_; }
  const ProcessControl<true>& latch() const { return latch_control_; }

  void TickProcess() override {
    Component::TickProcess();

    if (latch_control_.asserted()) {
      value_ = source_.value();
    }
  }

 private:
  ProcessControl<true> latch_control_;
  const ProgramCounter& source_;
  base::Word value_{};
};

}  // namespace irata2::sim

#endif  // IRATA2_SIM_LATCHED_WORD_REGISTER_H
