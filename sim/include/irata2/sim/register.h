#ifndef IRATA2_SIM_REGISTER_H
#define IRATA2_SIM_REGISTER_H

#include <utility>

#include "irata2/sim/component_with_bus.h"
#include "irata2/sim/control.h"

namespace irata2::sim {

template <typename Derived, typename ValueType>
class Register : public ComponentWithBus<Derived, ValueType> {
 public:
  using value_type = ValueType;

  Register(std::string name, Component& parent, Bus<ValueType>& bus)
      : ComponentWithBus<Derived, ValueType>(std::move(name), parent, bus),
        reset_control_("reset", *this) {}

  const ValueType& value() const { return value_; }
  void set_value(ValueType value) { value_ = value; }

  ProcessControl<true>& reset() { return reset_control_; }
  const ProcessControl<true>& reset() const { return reset_control_; }

  void TickWrite() override {
    if (this->write().asserted()) {
      this->bus().Write(value_, this->path());
    }
  }

  void TickRead() override {
    if (this->read().asserted()) {
      value_ = this->bus().Read(this->path());
    }
  }

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

#endif  // IRATA2_SIM_REGISTER_H
