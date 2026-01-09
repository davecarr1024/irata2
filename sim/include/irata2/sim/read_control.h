#ifndef IRATA2_SIM_READ_CONTROL_H
#define IRATA2_SIM_READ_CONTROL_H

#include <utility>

#include "irata2/sim/bus.h"
#include "irata2/sim/control.h"

namespace irata2::sim {

template <typename ValueType>
class ReadControl final : public Control<base::TickPhase::Read, true> {
 public:
  ReadControl(std::string name, Component& parent, const Bus<ValueType>& bus)
      : Control<base::TickPhase::Read, true>(std::move(name), parent),
        bus_(bus) {}

  const Bus<ValueType>& bus() const { return bus_; }

 private:
  const Bus<ValueType>& bus_;
};

}  // namespace irata2::sim

#endif  // IRATA2_SIM_READ_CONTROL_H
