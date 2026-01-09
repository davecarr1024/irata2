#ifndef IRATA2_HDL_MEMORY_ADDRESS_REGISTER_H
#define IRATA2_HDL_MEMORY_ADDRESS_REGISTER_H

#include "irata2/base/types.h"
#include "irata2/hdl/byte_bus.h"
#include "irata2/hdl/byte_register.h"
#include "irata2/hdl/register.h"
#include "irata2/hdl/word_bus.h"

#include <utility>

namespace irata2::hdl {

class MemoryAddressRegister final
    : public Register<MemoryAddressRegister, base::Word> {
 public:
  MemoryAddressRegister(std::string name,
                        ComponentBase& parent,
                        const WordBus& address_bus,
                        const ByteBus& data_bus)
      : Register<MemoryAddressRegister, base::Word>(std::move(name), parent, address_bus),
        low_("low", *this, data_bus),
        high_("high", *this, data_bus) {}

  const ByteRegister& low() const { return low_; }
  const ByteRegister& high() const { return high_; }

  template <typename Visitor>
  void visit_impl(Visitor&& visitor) const {
    Register<MemoryAddressRegister, base::Word>::visit_impl(visitor);
    low_.visit(visitor);
    high_.visit(visitor);
  }

 private:
  const ByteRegister low_;
  const ByteRegister high_;
};

}  // namespace irata2::hdl

#endif  // IRATA2_HDL_MEMORY_ADDRESS_REGISTER_H
