#ifndef IRATA2_SIM_MEMORY_ADDRESS_REGISTER_H
#define IRATA2_SIM_MEMORY_ADDRESS_REGISTER_H

#include <utility>

#include "irata2/base/types.h"
#include "irata2/sim/bus.h"
#include "irata2/sim/byte_register.h"
#include "irata2/sim/component.h"
#include "irata2/sim/control.h"
#include "irata2/sim/read_control.h"
#include "irata2/sim/register_with_bus.h"
#include "irata2/sim/write_control.h"

namespace irata2::sim::memory {

/**
 * @brief Memory Address Register with indexed addressing support.
 *
 * The MAR holds a 16-bit address for memory operations. It supports:
 * - Word read/write from address bus (inherited from RegisterWithBus)
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
class MemoryAddressRegister final : public RegisterWithBus<MemoryAddressRegister, base::Word> {
 public:
  class BytePort final : public ComponentWithParent {
   public:
    BytePort(std::string name,
             Component& parent,
             Bus<base::Byte>& data_bus,
             MemoryAddressRegister& mar,
             bool is_high);

    ReadControl<base::Byte>& read() { return read_; }
    const ReadControl<base::Byte>& read() const { return read_; }
    WriteControl<base::Byte>& write() { return write_; }
    const WriteControl<base::Byte>& write() const { return write_; }
    ProcessControl<true>& reset() { return reset_; }
    const ProcessControl<true>& reset() const { return reset_; }

    void TickWrite() override;
    void TickRead() override;
    void TickProcess() override;

   private:
    base::Byte CurrentValue() const;
    void SetValue(base::Byte value);

    ReadControl<base::Byte> read_;
    WriteControl<base::Byte> write_;
    ProcessControl<true> reset_;
    Bus<base::Byte>& data_bus_;
    MemoryAddressRegister& mar_;
    bool is_high_;
  };

  MemoryAddressRegister(std::string name,
                        Component& parent,
                        Bus<base::Word>& address_bus,
                        Bus<base::Byte>& data_bus);

  BytePort& low() { return low_; }
  const BytePort& low() const { return low_; }
  BytePort& high() { return high_; }
  const BytePort& high() const { return high_; }
  ByteRegister& offset() { return offset_; }
  const ByteRegister& offset() const { return offset_; }
  ProcessControl<true>& add_offset() { return add_offset_control_; }
  const ProcessControl<true>& add_offset() const { return add_offset_control_; }
  ProcessControl<true>& increment() { return increment_control_; }
  const ProcessControl<true>& increment() const { return increment_control_; }
  ProcessControl<true>& stack_page() { return stack_page_control_; }
  const ProcessControl<true>& stack_page() const { return stack_page_control_; }
  ProcessControl<true>& interrupt_vector() { return interrupt_vector_control_; }
  const ProcessControl<true>& interrupt_vector() const { return interrupt_vector_control_; }

  void TickProcess() override;

 private:
  base::Byte LowValue() const;
  base::Byte HighValue() const;
  void SetLow(base::Byte byte);
  void SetHigh(base::Byte byte);

  Bus<base::Byte>& data_bus_;
  BytePort low_;
  BytePort high_;
  ByteRegister offset_;
  ProcessControl<true> add_offset_control_;
  ProcessControl<true> increment_control_;
  ProcessControl<true> stack_page_control_;
  ProcessControl<true> interrupt_vector_control_;
};

}  // namespace irata2::sim::memory

#endif  // IRATA2_SIM_MEMORY_ADDRESS_REGISTER_H
