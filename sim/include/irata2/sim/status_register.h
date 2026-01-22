#ifndef IRATA2_SIM_STATUS_REGISTER_H
#define IRATA2_SIM_STATUS_REGISTER_H

#include <utility>

#include "irata2/sim/byte_register.h"
#include "irata2/sim/status.h"

namespace irata2::sim {

class StatusAnalyzer final : public ByteRegister {
 public:
  StatusAnalyzer(std::string name,
                 Component& parent,
                 Bus<base::Byte>& bus,
                 Status& zero,
                 Status& negative)
      : ByteRegister(std::move(name), parent, bus),
        zero_(zero),
        negative_(negative) {}

  void TickRead() override {
    ByteRegister::TickRead();
    if (!read().asserted()) {
      return;
    }
    const uint8_t value = this->value().value();
    zero_.Set(value == 0);
    negative_.Set((value & 0x80u) != 0);
  }

 private:
  Status& zero_;
  Status& negative_;
};

class StatusRegister final : public ByteRegister {
 public:
  StatusRegister(std::string name, Component& parent, Bus<base::Byte>& bus)
      : ByteRegister(std::move(name), parent, bus),
        negative_("negative", *this, *this, 7),
        overflow_("overflow", *this, *this, 6),
        unused_("unused", *this, *this, 5),
        brk_("break", *this, *this, 4),
        decimal_("decimal", *this, *this, 3),
        interrupt_disable_("interrupt_disable", *this, *this, 2),
        zero_("zero", *this, *this, 1),
        carry_("carry", *this, *this, 0),
        analyzer_("analyzer", *this, bus, zero_, negative_) {}

  Status& negative() { return negative_; }
  const Status& negative() const { return negative_; }
  Status& overflow() { return overflow_; }
  const Status& overflow() const { return overflow_; }
  Status& unused() { return unused_; }
  const Status& unused() const { return unused_; }
  Status& brk() { return brk_; }
  const Status& brk() const { return brk_; }
  Status& decimal() { return decimal_; }
  const Status& decimal() const { return decimal_; }
  Status& interrupt_disable() { return interrupt_disable_; }
  const Status& interrupt_disable() const { return interrupt_disable_; }
  Status& zero() { return zero_; }
  const Status& zero() const { return zero_; }
  Status& carry() { return carry_; }
  const Status& carry() const { return carry_; }
  StatusAnalyzer& analyzer() { return analyzer_; }
  const StatusAnalyzer& analyzer() const { return analyzer_; }

 private:
  Status negative_;
  Status overflow_;
  Status unused_;
  Status brk_;
  Status decimal_;
  Status interrupt_disable_;
  Status zero_;
  Status carry_;
  StatusAnalyzer analyzer_;
};

}  // namespace irata2::sim

#endif  // IRATA2_SIM_STATUS_REGISTER_H
