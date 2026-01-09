#ifndef IRATA2_HDL_REGISTER_H
#define IRATA2_HDL_REGISTER_H

#include "irata2/hdl/component_with_bus.h"
#include "irata2/hdl/traits.h"

#include <utility>

namespace irata2::hdl {

template <typename Derived, typename ValueType>
class Register : public ComponentWithBus<Derived, ValueType>, public RegisterTag {
 public:
  using value_type = ValueType;

  Register(std::string name, ComponentBase& parent, const Bus<ValueType>& bus)
      : ComponentWithBus<Derived, ValueType>(std::move(name), parent, bus) {}
};

}  // namespace irata2::hdl

#endif  // IRATA2_HDL_REGISTER_H
