#ifndef IRATA2_SIM_COMPONENT_WITH_BUS_H
#define IRATA2_SIM_COMPONENT_WITH_BUS_H

#include <utility>

#include "irata2/sim/bus.h"
#include "irata2/sim/component.h"
#include "irata2/sim/read_control.h"
#include "irata2/sim/write_control.h"

namespace irata2::sim {

/**
 * @brief Base class for components connected to a bus.
 *
 * ComponentWithBus provides a common pattern for bus-connected components:
 * - Write control: component writes its value to the bus
 * - Read control: component reads its value from the bus
 * - Automatic tick handling: TickWrite/TickRead check controls and call
 *   read_value()/write_value()
 *
 * Subclasses implement read_value() and write_value() to define how they
 * interact with the bus.
 *
 * @tparam Derived CRTP derived class
 * @tparam ValueType Type of value on the bus (Byte or Word)
 */
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

  /**
   * @brief Write phase: write component value to bus if control asserted.
   */
  void TickWrite() override {
    if (write_control_.asserted()) {
      bus_.Write(read_value(), path());
    }
  }

  /**
   * @brief Read phase: read value from bus if control asserted.
   */
  void TickRead() override {
    if (read_control_.asserted()) {
      write_value(bus_.Read(path()));
    }
  }

 protected:
  /**
   * @brief Read the component's current value to write to the bus.
   *
   * Called by TickWrite() when write control is asserted.
   * Subclasses implement this to provide their current value.
   */
  virtual ValueType read_value() const = 0;

  /**
   * @brief Write a value read from the bus to the component.
   *
   * Called by TickRead() when read control is asserted.
   * Subclasses implement this to store the value.
   */
  virtual void write_value(ValueType value) = 0;

 private:
  WriteControl<ValueType> write_control_;
  ReadControl<ValueType> read_control_;
  Bus<ValueType>& bus_;
};

}  // namespace irata2::sim

#endif  // IRATA2_SIM_COMPONENT_WITH_BUS_H
