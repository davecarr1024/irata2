#ifndef IRATA2_SIM_MEMORY_ADDRESS_REGISTER_H
#define IRATA2_SIM_MEMORY_ADDRESS_REGISTER_H

#include <utility>

#include "irata2/base/types.h"
#include "irata2/sim/bus.h"
#include "irata2/sim/component.h"
#include "irata2/sim/control.h"
#include "irata2/sim/read_control.h"
#include "irata2/sim/register.h"
#include "irata2/sim/write_control.h"

namespace irata2::sim::memory {

class MemoryAddressRegister final : public Register<MemoryAddressRegister, base::Word> {
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

 private:
  base::Byte LowValue() const;
  base::Byte HighValue() const;
  void SetLow(base::Byte byte);
  void SetHigh(base::Byte byte);

  Bus<base::Byte>& data_bus_;
  BytePort low_;
  BytePort high_;
};

}  // namespace irata2::sim::memory

#endif  // IRATA2_SIM_MEMORY_ADDRESS_REGISTER_H
