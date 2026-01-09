#ifndef IRATA2_HDL_BUS_H
#define IRATA2_HDL_BUS_H

#include "irata2/hdl/component_with_parent.h"
#include "irata2/hdl/traits.h"

#include <cstddef>
#include <utility>

namespace irata2::hdl {

template <typename ValueType>
class Bus : public ComponentWithParent<Bus<ValueType>>, public BusTag {
 public:
  using value_type = ValueType;
  static constexpr size_t kWidth = sizeof(ValueType) * 8;

  Bus(std::string name, ComponentBase& parent)
      : ComponentWithParent<Bus<ValueType>>(std::move(name), parent) {}

  template <typename Visitor>
  void visit_impl(Visitor&& visitor) const {
    visitor(*this);
  }
};

}  // namespace irata2::hdl

#endif  // IRATA2_HDL_BUS_H
