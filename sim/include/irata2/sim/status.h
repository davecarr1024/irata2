#ifndef IRATA2_SIM_STATUS_H
#define IRATA2_SIM_STATUS_H

#include "irata2/sim/component.h"
#include "irata2/sim/control.h"

#include <cstdint>

namespace irata2::sim {

class StatusRegister;

class Status final : public ComponentWithParent {
 public:
  Status(std::string name, Component& parent, StatusRegister& status, uint8_t bit_index);

  uint8_t bit_index() const { return bit_index_; }
  bool value() const;
  void Set(bool value);

  ProcessControl<true>& set() { return set_control_; }
  const ProcessControl<true>& set() const { return set_control_; }
  ProcessControl<true>& clear() { return clear_control_; }
  const ProcessControl<true>& clear() const { return clear_control_; }

  void TickProcess() override;

 private:
  StatusRegister& status_;
  uint8_t bit_index_;
  ProcessControl<true> set_control_;
  ProcessControl<true> clear_control_;
};

}  // namespace irata2::sim

#endif  // IRATA2_SIM_STATUS_H
