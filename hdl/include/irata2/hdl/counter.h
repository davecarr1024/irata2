#ifndef IRATA2_HDL_COUNTER_H
#define IRATA2_HDL_COUNTER_H

#include "irata2/hdl/bus.h"
#include "irata2/hdl/component_with_bus.h"
#include "irata2/hdl/process_control.h"
#include "irata2/hdl/register.h"

#include <utility>

namespace irata2::hdl {

template <typename ValueType>
class Counter final : public Register<Counter<ValueType>, ValueType> {
 public:
  Counter(std::string name, ComponentBase& parent, const Bus<ValueType>& bus)
      : Register<Counter<ValueType>, ValueType>(std::move(name), parent, bus),
        increment_control_("increment", *this) {}

  const ProcessControl<true>& increment() const { return increment_control_; }

  template <typename Visitor>
  void visit_impl(Visitor&& visitor) const {
    ComponentWithBus<Counter<ValueType>, ValueType>::visit_impl(visitor);
    increment_control_.visit(visitor);
  }

 private:
  const ProcessControl<true> increment_control_;
};

}  // namespace irata2::hdl

#endif  // IRATA2_HDL_COUNTER_H
