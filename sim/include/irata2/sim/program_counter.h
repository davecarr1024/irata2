#ifndef IRATA2_SIM_PROGRAM_COUNTER_H
#define IRATA2_SIM_PROGRAM_COUNTER_H

#include <optional>
#include <utility>

#include "irata2/base/types.h"
#include "irata2/sim/bus.h"
#include "irata2/sim/component_with_bus.h"
#include "irata2/sim/control.h"
#include "irata2/sim/read_control.h"
#include "irata2/sim/register_with_bus.h"

namespace irata2::sim {

/// Program counter that supports relative offset updates from the data bus.
class ProgramCounter final : public RegisterWithBus<ProgramCounter, base::Word> {
 public:
  ProgramCounter(std::string name,
                 Component& parent,
                 Bus<base::Word>& address_bus,
                 Bus<base::Byte>& data_bus)
      : RegisterWithBus<ProgramCounter, base::Word>(std::move(name), parent, address_bus),
        increment_control_("increment", *this),
        add_offset_control_("add_offset", *this, data_bus),
        data_bus_(data_bus) {}

  ProcessControl<true>& increment() { return increment_control_; }
  const ProcessControl<true>& increment() const { return increment_control_; }
  ReadControl<base::Byte>& add_offset() { return add_offset_control_; }
  const ReadControl<base::Byte>& add_offset() const { return add_offset_control_; }

  void TickRead() override {
    ComponentWithBus<ProgramCounter, base::Word>::TickRead();

    if (add_offset_control_.asserted()) {
      const base::Byte offset = data_bus_.Read(path());
      pending_offset_ = static_cast<int8_t>(offset.value());
    }
  }

  void TickProcess() override {
    Component::TickProcess();

    if (reset().asserted()) {
      value_mutable() = base::Word{};
      pending_offset_.reset();
      return;
    }

    if (increment_control_.asserted()) {
      value_mutable() = value() + base::Word{1};
    }

    if (pending_offset_.has_value()) {
      const int32_t signed_offset = static_cast<int32_t>(*pending_offset_);
      const int32_t updated = static_cast<int32_t>(value().value()) + signed_offset;
      uint32_t wrapped = static_cast<uint32_t>(updated) & 0xFFFFu;
      value_mutable() = base::Word{static_cast<uint16_t>(wrapped)};
      pending_offset_.reset();
    }
  }

 private:
  ProcessControl<true> increment_control_;
  ReadControl<base::Byte> add_offset_control_;
  Bus<base::Byte>& data_bus_;
  std::optional<int8_t> pending_offset_;
};

}  // namespace irata2::sim

#endif  // IRATA2_SIM_PROGRAM_COUNTER_H
