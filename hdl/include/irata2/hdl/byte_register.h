#ifndef IRATA2_HDL_BYTE_REGISTER_H
#define IRATA2_HDL_BYTE_REGISTER_H

#include "irata2/base/types.h"
#include "irata2/hdl/register.h"

#include <utility>

namespace irata2::hdl {

class ByteRegister final : public Register<ByteRegister, base::Byte> {
 public:
  ByteRegister(std::string name, ComponentBase& parent, const Bus<base::Byte>& bus)
      : Register<ByteRegister, base::Byte>(std::move(name), parent, bus) {}
};

}  // namespace irata2::hdl

#endif  // IRATA2_HDL_BYTE_REGISTER_H
