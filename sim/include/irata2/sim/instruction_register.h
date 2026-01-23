#ifndef IRATA2_SIM_INSTRUCTION_REGISTER_H
#define IRATA2_SIM_INSTRUCTION_REGISTER_H

#include <cstdint>
#include <utility>

#include "irata2/base/types.h"
#include "irata2/sim/byte_register.h"
#include "irata2/sim/control.h"
#include "irata2/sim/status.h"

namespace irata2::sim {

class InstructionRegister final : public ByteRegister {
 public:
  static constexpr uint8_t kIrqOpcode = 0x00;

  InstructionRegister(std::string name,
                      Component& parent,
                      Bus<base::Byte>& bus,
                      const LatchedProcessControl& irq_line,
                      const ProcessControl<true>& instruction_start,
                      const Status& interrupt_disable)
      : ByteRegister(std::move(name), parent, bus),
        irq_line_(irq_line),
        instruction_start_(instruction_start),
        interrupt_disable_(interrupt_disable) {}

  base::Byte value() const {
    if (inject_interrupt_) {
      return base::Byte{kIrqOpcode};
    }
    return ByteRegister::value();
  }

  void TickProcess() override {
    ByteRegister::TickProcess();

    if (reset().asserted()) {
      inject_interrupt_ = false;
      return;
    }

    if (instruction_start_.asserted()) {
      inject_interrupt_ = irq_line_.asserted() && !interrupt_disable_.value();
    }
  }

 private:
  const LatchedProcessControl& irq_line_;
  const ProcessControl<true>& instruction_start_;
  const Status& interrupt_disable_;
  bool inject_interrupt_ = false;
};

}  // namespace irata2::sim

#endif  // IRATA2_SIM_INSTRUCTION_REGISTER_H
