#ifndef IRATA2_SIM_REGISTER_WITH_BUS_H
#define IRATA2_SIM_REGISTER_WITH_BUS_H

#include <utility>

#include "irata2/sim/component_with_bus.h"
#include "irata2/sim/control.h"
#include "irata2/sim/register_base.h"

namespace irata2::sim {

/// Register template for registers connected to a bus.
///
/// This extends RegisterBase to add bus connectivity:
/// - Inherits value storage and reset control from RegisterBase
/// - Adds read/write controls for bus operations
/// - Implements ComponentWithBus interface
///
/// @tparam Derived The derived class type (CRTP pattern)
/// @tparam ValueType The type of value stored and transferred on the bus
template <typename Derived, typename ValueType>
class RegisterWithBus : public ComponentWithBus<Derived, ValueType> {
 public:
  using value_type = ValueType;

  RegisterWithBus(std::string name, Component& parent, Bus<ValueType>& bus)
      : ComponentWithBus<Derived, ValueType>(std::move(name), parent, bus),
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
  // Implement ComponentWithBus abstract interface
  ValueType read_value() const override { return value_; }
  void write_value(ValueType value) override { value_ = value; }

  ValueType& value_mutable() { return value_; }

 private:
  ProcessControl<true> reset_control_;
  ValueType value_{};
};

}  // namespace irata2::sim

#endif  // IRATA2_SIM_REGISTER_WITH_BUS_H
