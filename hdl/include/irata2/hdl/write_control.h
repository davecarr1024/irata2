#ifndef IRATA2_HDL_WRITE_CONTROL_H
#define IRATA2_HDL_WRITE_CONTROL_H

#include "irata2/hdl/bus.h"
#include "irata2/hdl/control.h"

#include <utility>

namespace irata2::hdl {

template <typename ValueType>
class WriteControl final
    : public Control<WriteControl<ValueType>, ValueType,
                     base::TickPhase::Write, true> {
 public:
  WriteControl(std::string name, ComponentBase& parent, const Bus<ValueType>& bus)
      : Control<WriteControl<ValueType>, ValueType, base::TickPhase::Write, true>(
            std::move(name), parent),
        bus_(bus) {}

  const Bus<ValueType>& bus() const { return bus_; }

 private:
  const Bus<ValueType>& bus_;
};

}  // namespace irata2::hdl

#endif  // IRATA2_HDL_WRITE_CONTROL_H
