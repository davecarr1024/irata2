#ifndef IRATA2_HDL_COMPONENT_WITH_BUS_H
#define IRATA2_HDL_COMPONENT_WITH_BUS_H

#include "irata2/hdl/bus.h"
#include "irata2/hdl/component_with_parent.h"
#include "irata2/hdl/read_control.h"
#include "irata2/hdl/write_control.h"

#include <utility>

namespace irata2::hdl {

template <typename Derived, typename ValueType>
class ComponentWithBus : public ComponentWithParent<Derived> {
 public:
  ComponentWithBus(std::string name, ComponentBase& parent,
                   const Bus<ValueType>& bus)
      : ComponentWithParent<Derived>(std::move(name), parent),
        write_control_("write", *this, bus),
        read_control_("read", *this, bus),
        bus_(bus) {}

  const WriteControl<ValueType>& write() const { return write_control_; }
  const ReadControl<ValueType>& read() const { return read_control_; }
  const Bus<ValueType>& bus() const { return bus_; }

  template <typename Visitor>
  void visit_impl(Visitor&& visitor) const {
    visitor(static_cast<const Derived&>(*this));
    write_control_.visit(visitor);
    read_control_.visit(visitor);
  }

 private:
  const WriteControl<ValueType> write_control_;
  const ReadControl<ValueType> read_control_;
  const Bus<ValueType>& bus_;
};

}  // namespace irata2::hdl

#endif  // IRATA2_HDL_COMPONENT_WITH_BUS_H
