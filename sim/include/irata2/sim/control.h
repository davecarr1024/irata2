#ifndef IRATA2_SIM_CONTROL_H
#define IRATA2_SIM_CONTROL_H

#include <string>

#include "irata2/base/tick_phase.h"
#include "irata2/sim/component.h"
#include "irata2/sim/error.h"

namespace irata2::sim {

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

template <base::TickPhase Phase, bool AutoReset>
class Control : public ControlBase {
 public:
  Control(std::string name, Component& parent)
      : ControlBase(std::move(name), parent, Phase, AutoReset) {}
};

template <bool AutoReset = true>
using ProcessControl = Control<base::TickPhase::Process, AutoReset>;

}  // namespace irata2::sim

#endif  // IRATA2_SIM_CONTROL_H
