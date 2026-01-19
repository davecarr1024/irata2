#ifndef IRATA2_SIM_PROGRAM_COUNTER_H
#define IRATA2_SIM_PROGRAM_COUNTER_H

#include <utility>

#include "irata2/base/types.h"
#include "irata2/sim/byte_register.h"
#include "irata2/sim/bus.h"
#include "irata2/sim/component_with_bus.h"
#include "irata2/sim/control.h"
#include "irata2/sim/register_with_bus.h"

namespace irata2::sim {

/// Program counter with explicit signed offset register for relative branches.
///
/// The signed_offset child register holds the branch displacement read from
/// the data bus. The add_signed_offset control adds this value (interpreted
/// as signed) to the PC during the Process phase.
class ProgramCounter final : public RegisterWithBus<ProgramCounter, base::Word> {
 public:
  class BytePort final : public ComponentWithParent {
   public:
    BytePort(std::string name,
             Component& parent,
             Bus<base::Byte>& data_bus,
             ProgramCounter& pc,
             bool is_high)
        : ComponentWithParent(parent, std::move(name)),
          read_("read", *this, data_bus),
          write_("write", *this, data_bus),
          reset_("reset", *this),
          data_bus_(data_bus),
          pc_(pc),
          is_high_(is_high) {}

    ReadControl<base::Byte>& read() { return read_; }
    const ReadControl<base::Byte>& read() const { return read_; }
    WriteControl<base::Byte>& write() { return write_; }
    const WriteControl<base::Byte>& write() const { return write_; }
    ProcessControl<true>& reset() { return reset_; }
    const ProcessControl<true>& reset() const { return reset_; }

    void TickWrite() override {
      if (write_.asserted()) {
        data_bus_.Write(CurrentValue(), path());
      }
    }

    void TickRead() override {
      if (read_.asserted()) {
        SetValue(data_bus_.Read(path()));
      }
    }

    void TickProcess() override {
      if (reset_.asserted()) {
        SetValue(base::Byte{0});
      }
    }

   private:
    base::Byte CurrentValue() const {
      return is_high_ ? pc_.value().high() : pc_.value().low();
    }

    void SetValue(base::Byte value) {
      if (is_high_) {
        pc_.set_value(base::Word(value, pc_.value().low()));
      } else {
        pc_.set_value(base::Word(pc_.value().high(), value));
      }
    }

    ReadControl<base::Byte> read_;
    WriteControl<base::Byte> write_;
    ProcessControl<true> reset_;
    Bus<base::Byte>& data_bus_;
    ProgramCounter& pc_;
    bool is_high_;
  };

  ProgramCounter(std::string name,
                 Component& parent,
                 Bus<base::Word>& address_bus,
                 Bus<base::Byte>& data_bus)
      : RegisterWithBus<ProgramCounter, base::Word>(std::move(name), parent, address_bus),
        low_("low", *this, data_bus, *this, false),
        high_("high", *this, data_bus, *this, true),
        signed_offset_("signed_offset", *this, data_bus),
        increment_control_("increment", *this),
        add_signed_offset_control_("add_signed_offset", *this) {}

  BytePort& low() { return low_; }
  const BytePort& low() const { return low_; }
  BytePort& high() { return high_; }
  const BytePort& high() const { return high_; }
  ByteRegister& signed_offset() { return signed_offset_; }
  const ByteRegister& signed_offset() const { return signed_offset_; }
  ProcessControl<true>& increment() { return increment_control_; }
  const ProcessControl<true>& increment() const { return increment_control_; }
  ProcessControl<true>& add_signed_offset() { return add_signed_offset_control_; }
  const ProcessControl<true>& add_signed_offset() const { return add_signed_offset_control_; }

  void TickProcess() override {
    Component::TickProcess();

    if (reset().asserted()) {
      value_mutable() = base::Word{};
      return;
    }

    if (increment_control_.asserted()) {
      value_mutable() = value() + base::Word{1};
    }

    if (add_signed_offset_control_.asserted()) {
      const int8_t offset = static_cast<int8_t>(signed_offset_.value().value());
      const int32_t updated = static_cast<int32_t>(value().value()) + offset;
      uint32_t wrapped = static_cast<uint32_t>(updated) & 0xFFFFu;
      value_mutable() = base::Word{static_cast<uint16_t>(wrapped)};
    }
  }

 private:
  BytePort low_;
  BytePort high_;
  ByteRegister signed_offset_;
  ProcessControl<true> increment_control_;
  ProcessControl<true> add_signed_offset_control_;
};

}  // namespace irata2::sim

#endif  // IRATA2_SIM_PROGRAM_COUNTER_H
