#ifndef IRATA2_SIM_STATUS_REGISTER_H
#define IRATA2_SIM_STATUS_REGISTER_H

#include <utility>

#include "irata2/sim/byte_register.h"

namespace irata2::sim {

class StatusRegister final : public ByteRegister {
 public:
  StatusRegister(std::string name, Component& parent, Bus<base::Byte>& bus)
      : ByteRegister(std::move(name), parent, bus) {}
};

}  // namespace irata2::sim

#endif  // IRATA2_SIM_STATUS_REGISTER_H
