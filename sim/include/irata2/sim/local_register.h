#ifndef IRATA2_SIM_LOCAL_REGISTER_H
#define IRATA2_SIM_LOCAL_REGISTER_H

#include <utility>

#include "irata2/sim/component.h"
#include "irata2/sim/control.h"

namespace irata2::sim {

template <typename Derived, typename ValueType>
class LocalRegister : public ComponentWithParent {
 public:
  LocalRegister(std::string name, Component& parent)
      : ComponentWithParent(parent, std::move(name)),
        reset_control_("reset", *this) {}

  const ValueType& value() const { return value_; }
  void set_value(ValueType value) { value_ = value; }

  ProcessControl<true>& reset() { return reset_control_; }
  const ProcessControl<true>& reset() const { return reset_control_; }

  void TickProcess() override {
    if (reset_control_.asserted()) {
      value_ = ValueType{};
    }
  }

 protected:
  ValueType& value_mutable() { return value_; }

 private:
  ProcessControl<true> reset_control_;
  ValueType value_{};
};

}  // namespace irata2::sim

#endif  // IRATA2_SIM_LOCAL_REGISTER_H
