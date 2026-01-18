#ifndef IRATA2_HDL_MEMORY_ADDRESS_REGISTER_H
#define IRATA2_HDL_MEMORY_ADDRESS_REGISTER_H

#include "irata2/base/types.h"
#include "irata2/hdl/byte_bus.h"
#include "irata2/hdl/byte_register.h"
#include "irata2/hdl/process_control.h"
#include "irata2/hdl/register.h"
#include "irata2/hdl/word_bus.h"

#include <utility>

namespace irata2::hdl {

/**
 * @brief Memory Address Register with indexed addressing support.
 *
 * The MAR holds a 16-bit address for memory operations. It supports:
 * - Word read/write from address bus (inherited from Register)
 * - Byte read/write for low/high bytes from data bus
 * - Indexed addressing via offset register and add_offset control
 *
 * For indexed addressing (ZPX, ZPY, ABX, ABY), microcode loads the index
 * register value into the offset register, then asserts add_offset. The
 * addition is unsigned with carry from low to high byte.
 *
 * For zero page indexed modes, microcode should assert mar.high.reset after
 * add_offset to wrap within zero page ($00-$FF).
 */
class MemoryAddressRegister final
    : public Register<MemoryAddressRegister, base::Word> {
 public:
  MemoryAddressRegister(std::string name,
                        ComponentBase& parent,
                        const WordBus& address_bus,
                        const ByteBus& data_bus)
      : Register<MemoryAddressRegister, base::Word>(std::move(name), parent, address_bus),
        low_("low", *this, data_bus),
        high_("high", *this, data_bus),
        offset_("offset", *this, data_bus),
        add_offset_control_("add_offset", *this) {}

  const ByteRegister& low() const { return low_; }
  const ByteRegister& high() const { return high_; }
  const ByteRegister& offset() const { return offset_; }
  const ProcessControl<true>& add_offset() const { return add_offset_control_; }

  template <typename Visitor>
  void visit_impl(Visitor&& visitor) const {
    Register<MemoryAddressRegister, base::Word>::visit_impl(visitor);
    low_.visit(visitor);
    high_.visit(visitor);
    offset_.visit(visitor);
    add_offset_control_.visit(visitor);
  }

 private:
  const ByteRegister low_;
  const ByteRegister high_;
  const ByteRegister offset_;
  const ProcessControl<true> add_offset_control_;
};

}  // namespace irata2::hdl

#endif  // IRATA2_HDL_MEMORY_ADDRESS_REGISTER_H
