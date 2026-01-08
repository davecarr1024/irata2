#ifndef IRATA2_SIM_CPU_H
#define IRATA2_SIM_CPU_H

#include "irata2/base/tick_phase.h"
#include "irata2/hdl/cpu.h"
#include "irata2/sim/component.h"

namespace irata2::sim {

// Root simulator component representing the CPU runtime
// This has runtime state and orchestrates the five-phase tick model
class Cpu : public Component {
 public:
  explicit Cpu(const hdl::Cpu& hdl);

  Cpu& cpu() override { return *this; }
  const Cpu& cpu() const override { return *this; }

  std::string path() const override { return "/cpu"; }

  // Get current tick phase
  base::TickPhase current_phase() const { return current_phase_; }

  // Execute one clock cycle (five phases)
  void Tick();

  // Halt state
  bool halted() const { return halted_; }
  void set_halted(bool halted) { halted_ = halted; }

  // Cycle count
  uint64_t cycle_count() const { return cycle_count_; }

  // Component accessors will be added here as we build the system
  // For example:
  // ByteBus& data_bus() { return data_bus_; }
  // const ByteBus& data_bus() const { return data_bus_; }
  // ByteRegister& a() { return a_; }
  // const ByteRegister& a() const { return a_; }

 private:
  const hdl::Cpu& hdl_;
  base::TickPhase current_phase_ = base::TickPhase::None;
  bool halted_ = false;
  uint64_t cycle_count_ = 0;

  // Component members will be added here
  // ByteBus data_bus_;
  // WordBus address_bus_;
  // ByteRegister a_;
  // Memory memory_;
};

}  // namespace irata2::sim

#endif  // IRATA2_SIM_CPU_H
