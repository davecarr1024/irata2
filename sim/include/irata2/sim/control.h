#ifndef IRATA2_SIM_CONTROL_H
#define IRATA2_SIM_CONTROL_H

#include <string>

#include "irata2/base/tick_phase.h"
#include "irata2/sim/component.h"

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
  bool asserted() const { return asserted_; }

  void Set(bool asserted) { asserted_ = asserted; }
  void Assert() { asserted_ = true; }
  void Clear() { asserted_ = false; }

  void ClearIfAutoReset() {
    if (auto_reset_) {
      asserted_ = false;
    }
  }

  void TickClear() override { ClearIfAutoReset(); }

 private:
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
