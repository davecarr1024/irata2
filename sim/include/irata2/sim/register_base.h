#ifndef IRATA2_SIM_REGISTER_BASE_H
#define IRATA2_SIM_REGISTER_BASE_H

#include <utility>

#include "irata2/sim/component.h"
#include "irata2/sim/control.h"

namespace irata2::sim {

/// Base register template for registers NOT connected to a bus.
///
/// This is the foundation of the register hierarchy. Registers at this level:
/// - Store a value of type ValueType
/// - Have a reset control that clears the value to default
/// - Are NOT connected to any bus (no read/write controls)
///
/// For bus-connected registers, see RegisterWithBus.
///
/// @tparam Derived The derived class type (CRTP pattern)
/// @tparam ValueType The type of value stored (e.g., base::Byte, base::Word)
template <typename Derived, typename ValueType>
class RegisterBase : public ComponentWithParent {
 public:
  using value_type = ValueType;

  RegisterBase(std::string name, Component& parent)
      : ComponentWithParent(parent, std::move(name)),
        reset_control_("reset", *this) {}

  const ValueType& value() const { return value_; }
  void set_value(ValueType value) { value_ = value; }

  ProcessControl<true>& reset() { return reset_control_; }
  const ProcessControl<true>& reset() const { return reset_control_; }

  void TickProcess() override {
    // First propagate to children
    Component::TickProcess();

    // Then handle reset
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

#endif  // IRATA2_SIM_REGISTER_BASE_H
