#ifndef IRATA2_HDL_READ_CONTROL_H
#define IRATA2_HDL_READ_CONTROL_H

#include "irata2/hdl/bus.h"
#include "irata2/hdl/control.h"

#include <utility>

namespace irata2::hdl {

template <typename ValueType>
class ReadControl final
    : public Control<ReadControl<ValueType>, ValueType,
                     base::TickPhase::Read, true> {
 public:
  ReadControl(std::string name, ComponentBase& parent, const Bus<ValueType>& bus)
      : Control<ReadControl<ValueType>, ValueType, base::TickPhase::Read, true>(
            std::move(name), parent),
        bus_(bus) {}

  const Bus<ValueType>& bus() const { return bus_; }

 private:
  const Bus<ValueType>& bus_;
};

}  // namespace irata2::hdl

#endif  // IRATA2_HDL_READ_CONTROL_H
