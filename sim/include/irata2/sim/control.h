#ifndef IRATA2_SIM_CONTROL_H
#define IRATA2_SIM_CONTROL_H

#include <string>

#include "irata2/base/tick_phase.h"
#include "irata2/sim/component.h"
#include "irata2/sim/error.h"

namespace irata2::sim {

/**
 * @brief Base class for runtime control signals with phase validation.
 *
 * ControlBase provides phase-aware control signal semantics. Controls can
 * only be asserted during the Control phase, and their asserted() state
 * can only be read during their designated phase.
 *
 * This enforcement catches timing bugs that would be silent in real hardware,
 * where out-of-phase reads would return stale or undefined values.
 *
 * @code
 * // During Control phase:
 * control.Assert();  // OK
 *
 * // During Write phase:
 * control.Assert();  // Throws SimError!
 *
 * // During the control's designated phase:
 * if (control.asserted()) { ... }  // OK
 * @endcode
 *
 * @see Control for the typed template wrapper
 * @see ProcessControl for Process-phase controls
 */
class ControlBase : public ComponentWithParent {
 public:
  ControlBase(std::string name,
              Component& parent,
              base::TickPhase phase,
              bool auto_reset)
      : ComponentWithParent(parent, std::move(name)),
        phase_(phase),
        auto_reset_(auto_reset) {}

  base::TickPhase phase() const { return phase_; }
  bool auto_reset() const { return auto_reset_; }
  bool asserted() const {
    EnsurePhase(phase_, "read");
    return asserted_;
  }

  void Set(bool asserted) {
    EnsurePhase(base::TickPhase::Control, "set");
    asserted_ = asserted;
  }
  void Assert() {
    EnsurePhase(base::TickPhase::Control, "assert");
    asserted_ = true;
  }
  void Clear() {
    EnsurePhase(base::TickPhase::Control, "clear");
    asserted_ = false;
  }

  void ClearIfAutoReset() {
    if (auto_reset_) {
      asserted_ = false;
    }
  }

  void TickClear() override { ClearIfAutoReset(); }

 private:
  void EnsurePhase(base::TickPhase expected, std::string_view action) const {
    if (current_phase() != expected) {
      throw SimError("control " + std::string(action) + " outside " +
                     base::ToString(expected) + " phase: " + path());
    }
  }

  base::TickPhase phase_;
  bool auto_reset_;
  bool asserted_ = false;
};

/**
 * @brief Typed control signal with compile-time phase and auto-reset.
 *
 * @tparam Phase The tick phase during which this control is readable
 * @tparam AutoReset If true, control clears automatically in Clear phase
 */
template <base::TickPhase Phase, bool AutoReset>
class Control : public ControlBase {
 public:
  Control(std::string name, Component& parent)
      : ControlBase(std::move(name), parent, Phase, AutoReset) {}
};

/**
 * @brief Control signal readable during Process phase.
 *
 * Used for signals like halt and crash that are checked during
 * the Process phase to update CPU state.
 *
 * @tparam AutoReset If true (default), control clears after each tick
 */
template <bool AutoReset = true>
using ProcessControl = Control<base::TickPhase::Process, AutoReset>;

}  // namespace irata2::sim

#endif  // IRATA2_SIM_CONTROL_H
