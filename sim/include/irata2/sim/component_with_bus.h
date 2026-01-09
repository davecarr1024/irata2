#ifndef IRATA2_SIM_COMPONENT_WITH_BUS_H
#define IRATA2_SIM_COMPONENT_WITH_BUS_H

#include <utility>

#include "irata2/sim/bus.h"
#include "irata2/sim/component.h"
#include "irata2/sim/read_control.h"
#include "irata2/sim/write_control.h"

namespace irata2::sim {

template <typename Derived, typename ValueType>
class ComponentWithBus : public ComponentWithParent {
 public:
  ComponentWithBus(std::string name, Component& parent, Bus<ValueType>& bus)
      : ComponentWithParent(parent, std::move(name)),
        write_control_("write", *this, bus),
        read_control_("read", *this, bus),
        bus_(bus) {}

  WriteControl<ValueType>& write() { return write_control_; }
  const WriteControl<ValueType>& write() const { return write_control_; }

  ReadControl<ValueType>& read() { return read_control_; }
  const ReadControl<ValueType>& read() const { return read_control_; }

  Bus<ValueType>& bus() { return bus_; }
  const Bus<ValueType>& bus() const { return bus_; }

 private:
  WriteControl<ValueType> write_control_;
  ReadControl<ValueType> read_control_;
  Bus<ValueType>& bus_;
};

}  // namespace irata2::sim

#endif  // IRATA2_SIM_COMPONENT_WITH_BUS_H
