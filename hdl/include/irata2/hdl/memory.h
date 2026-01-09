#ifndef IRATA2_HDL_MEMORY_H
#define IRATA2_HDL_MEMORY_H

#include "irata2/base/types.h"
#include "irata2/hdl/byte_bus.h"
#include "irata2/hdl/component_with_bus.h"
#include "irata2/hdl/memory_address_register.h"
#include "irata2/hdl/word_bus.h"

#include <utility>

namespace irata2::hdl {

class Memory final : public ComponentWithBus<Memory, base::Byte> {
 public:
  Memory(std::string name,
         ComponentBase& parent,
         const ByteBus& data_bus,
         const WordBus& address_bus)
      : ComponentWithBus<Memory, base::Byte>(std::move(name), parent, data_bus),
        mar_("mar", *this, address_bus, data_bus) {}

  const MemoryAddressRegister& mar() const { return mar_; }

  template <typename Visitor>
  void visit_impl(Visitor&& visitor) const {
    ComponentWithBus<Memory, base::Byte>::visit_impl(visitor);
    mar_.visit(visitor);
  }

 private:
  const MemoryAddressRegister mar_;
};

}  // namespace irata2::hdl

#endif  // IRATA2_HDL_MEMORY_H
