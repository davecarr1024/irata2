#ifndef IRATA2_SIM_BYTE_REGISTER_H
#define IRATA2_SIM_BYTE_REGISTER_H

#include <utility>

#include "irata2/base/types.h"
#include "irata2/sim/register.h"

namespace irata2::sim {

class ByteRegister : public Register<ByteRegister, base::Byte> {
 public:
  ByteRegister(std::string name, Component& parent, Bus<base::Byte>& bus)
      : Register<ByteRegister, base::Byte>(std::move(name), parent, bus) {}
};

}  // namespace irata2::sim

#endif  // IRATA2_SIM_BYTE_REGISTER_H
