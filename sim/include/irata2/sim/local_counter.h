#ifndef IRATA2_SIM_LOCAL_COUNTER_H
#define IRATA2_SIM_LOCAL_COUNTER_H

#include <utility>

#include "irata2/sim/control.h"
#include "irata2/sim/local_register.h"

namespace irata2::sim {

template <typename ValueType>
class LocalCounter : public LocalRegister<LocalCounter<ValueType>, ValueType> {
 public:
  LocalCounter(std::string name, Component& parent)
      : LocalRegister<LocalCounter<ValueType>, ValueType>(std::move(name),
                                                          parent),
        increment_control_("increment", *this) {}

  ProcessControl<true>& increment() { return increment_control_; }
  const ProcessControl<true>& increment() const { return increment_control_; }

  void TickProcess() override {
    if (this->reset().asserted()) {
      this->value_mutable() = ValueType{};
      return;
    }
    if (increment_control_.asserted()) {
      this->value_mutable() = this->value() + ValueType{1};
    }
  }

 private:
  ProcessControl<true> increment_control_;
};

}  // namespace irata2::sim

#endif  // IRATA2_SIM_LOCAL_COUNTER_H
