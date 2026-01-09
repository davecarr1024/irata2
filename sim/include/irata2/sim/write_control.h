#ifndef IRATA2_SIM_WRITE_CONTROL_H
#define IRATA2_SIM_WRITE_CONTROL_H

#include <utility>

#include "irata2/sim/bus.h"
#include "irata2/sim/control.h"

namespace irata2::sim {

template <typename ValueType>
class WriteControl final : public Control<base::TickPhase::Write, true> {
 public:
  WriteControl(std::string name, Component& parent, const Bus<ValueType>& bus)
      : Control<base::TickPhase::Write, true>(std::move(name), parent),
        bus_(bus) {}

  const Bus<ValueType>& bus() const { return bus_; }

 private:
  const Bus<ValueType>& bus_;
};

}  // namespace irata2::sim

#endif  // IRATA2_SIM_WRITE_CONTROL_H
