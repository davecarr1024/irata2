#ifndef IRATA2_SIM_CONTROL_H
#define IRATA2_SIM_CONTROL_H

#include <string>
#include <type_traits>

#include "irata2/base/tick_phase.h"
#include "irata2/sim/component.h"
#include "irata2/sim/error.h"

namespace irata2::sim {

/**
 * @brief Abstract base class for all runtime control signals.
 *
 * ControlBase is the root of the control hierarchy. It provides phase-aware
 * control signal semantics: controls can only be asserted during the Control
 * phase, and their asserted() state can only be read during their designated
 * phase.
 *
 * This enforcement catches timing bugs that would be silent in real hardware,
 * where out-of-phase reads would return stale or undefined values.
 *
 * Hierarchy:
 * - AutoResetControl: Controls that clear automatically in Clear phase
 * - LatchedControl: Controls that remain asserted until explicitly cleared
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
 * @see AutoResetControl for automatically clearing controls
 * @see LatchedControl for persistent controls
 */
class ControlBase : public ComponentWithParent {
 public:
  ControlBase(std::string name, Component& parent, base::TickPhase phase)
      : ComponentWithParent(parent, std::move(name)), phase_(phase) {}

  virtual ~ControlBase() = default;

  base::TickPhase phase() const { return phase_; }

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

 protected:
  void EnsurePhase(base::TickPhase expected, std::string_view action) const {
    if (current_phase() != expected) {
      throw SimError("control " + std::string(action) + " outside " +
                     base::ToString(expected) + " phase: " + path());
    }
  }

  bool asserted_ = false;

 private:
  base::TickPhase phase_;
};

/**
 * @brief Intermediate class for controls that auto-reset in Clear phase.
 *
 * AutoResetControl provides automatic clearing behavior: the control is
 * cleared at the end of each tick cycle during the Clear phase. This is
 * the default behavior for most controls.
 *
 * @see LatchedControl for controls that persist across ticks
 */
class AutoResetControl : public ControlBase {
 public:
  AutoResetControl(std::string name, Component& parent, base::TickPhase phase)
      : ControlBase(std::move(name), parent, phase) {}

  void TickClear() override { asserted_ = false; }
};

/**
 * @brief Intermediate class for controls that persist until explicitly cleared.
 *
 * LatchedControl provides persistent behavior: the control remains asserted
 * across tick cycles until explicitly cleared via Clear() or Set(false).
 *
 * @see AutoResetControl for controls that clear automatically
 */
class LatchedControl : public ControlBase {
 public:
  LatchedControl(std::string name, Component& parent, base::TickPhase phase)
      : ControlBase(std::move(name), parent, phase) {}

  // TickClear intentionally does nothing - control persists
};

/**
 * @brief Typed control signal with compile-time phase and auto-reset.
 *
 * Control provides a convenient template wrapper around the control hierarchy,
 * selecting AutoResetControl or LatchedControl based on the template parameter.
 *
 * @tparam Phase The tick phase during which this control is readable
 * @tparam AutoReset If true, control clears automatically in Clear phase
 */
template <base::TickPhase Phase, bool AutoReset>
class Control
    : public std::conditional_t<AutoReset, AutoResetControl, LatchedControl> {
 public:
  using BaseType =
      std::conditional_t<AutoReset, AutoResetControl, LatchedControl>;

  Control(std::string name, Component& parent)
      : BaseType(std::move(name), parent, Phase) {}
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

using LatchedProcessControl = ProcessControl<false>;

}  // namespace irata2::sim

#endif  // IRATA2_SIM_CONTROL_H
