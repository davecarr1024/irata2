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
  ProgramCounter(std::string name,
                 Component& parent,
                 Bus<base::Word>& address_bus,
                 Bus<base::Byte>& data_bus)
      : RegisterWithBus<ProgramCounter, base::Word>(std::move(name), parent, address_bus),
        signed_offset_("signed_offset", *this, data_bus),
        increment_control_("increment", *this),
        add_signed_offset_control_("add_signed_offset", *this) {}

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
  ByteRegister signed_offset_;
  ProcessControl<true> increment_control_;
  ProcessControl<true> add_signed_offset_control_;
};

}  // namespace irata2::sim

#endif  // IRATA2_SIM_PROGRAM_COUNTER_H
