#ifndef IRATA2_SIM_BYTE_BUS_H
#define IRATA2_SIM_BYTE_BUS_H

#include <utility>

#include "irata2/base/types.h"
#include "irata2/sim/bus.h"

namespace irata2::sim {

class ByteBus final : public Bus<base::Byte> {
 public:
  ByteBus(std::string name, Component& parent)
      : Bus<base::Byte>(std::move(name), parent) {}
};

}  // namespace irata2::sim

#endif  // IRATA2_SIM_BYTE_BUS_H
