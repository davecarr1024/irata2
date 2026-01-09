#ifndef IRATA2_SIM_CPU_H
#define IRATA2_SIM_CPU_H

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "irata2/base/tick_phase.h"
#include "irata2/hdl/cpu.h"
#include "irata2/microcode/output/program.h"
#include "irata2/sim/byte_bus.h"
#include "irata2/sim/byte_register.h"
#include "irata2/sim/component.h"
#include "irata2/sim/control.h"
#include "irata2/sim/controller.h"
#include "irata2/sim/counter.h"
#include "irata2/sim/status_register.h"
#include "irata2/sim/word_bus.h"
#include "irata2/sim/word_register.h"

namespace irata2::sim {

// Root simulator component representing the CPU runtime
// This has runtime state and orchestrates the five-phase tick model
class Cpu : public Component {
 public:
  Cpu();
  explicit Cpu(std::shared_ptr<const hdl::Cpu> hdl,
               std::shared_ptr<const microcode::output::MicrocodeProgram> program);

  Cpu& cpu() override { return *this; }
  const Cpu& cpu() const override { return *this; }

  std::string path() const override { return ""; }

  // Get current tick phase
  base::TickPhase current_phase() const override { return current_phase_; }

  // Execute one clock cycle (five phases)
  void Tick();

  // Halt state
  bool halted() const { return halted_; }
  void set_halted(bool halted) { halted_ = halted; }

  // Cycle count
  uint64_t cycle_count() const { return cycle_count_; }

  bool crashed() const { return crashed_; }

  const hdl::Cpu& hdl() const { return *hdl_; }
  const microcode::output::MicrocodeProgram& microcode() const {
    return *microcode_;
  }

  ProcessControl<true>& halt() { return halt_control_; }
  const ProcessControl<true>& halt() const { return halt_control_; }
  ProcessControl<true>& crash() { return crash_control_; }
  const ProcessControl<true>& crash() const { return crash_control_; }

  ByteBus& data_bus() { return data_bus_; }
  const ByteBus& data_bus() const { return data_bus_; }
  WordBus& address_bus() { return address_bus_; }
  const WordBus& address_bus() const { return address_bus_; }

  ByteRegister& a() { return a_; }
  const ByteRegister& a() const { return a_; }
  ByteRegister& x() { return x_; }
  const ByteRegister& x() const { return x_; }
  Counter<base::Word>& pc() { return pc_; }
  const Counter<base::Word>& pc() const { return pc_; }
  WordRegister& mar() { return mar_; }
  const WordRegister& mar() const { return mar_; }
  StatusRegister& status() { return status_; }
  const StatusRegister& status() const { return status_; }
  Controller& controller() { return controller_; }
  const Controller& controller() const { return controller_; }

  ControlBase* ResolveControl(std::string_view path);
  const ControlBase* ResolveControl(std::string_view path) const;
  std::vector<std::string> AllControlPaths() const;

  void RegisterChild(Component& child) override;

 private:
  void IndexControls() const;
  void TickPhase(void (Component::*phase)());
  void TickProcess() override;

  std::shared_ptr<const hdl::Cpu> hdl_;
  std::shared_ptr<const microcode::output::MicrocodeProgram> microcode_;
  base::TickPhase current_phase_ = base::TickPhase::None;
  bool halted_ = false;
  bool crashed_ = false;
  uint64_t cycle_count_ = 0;

  std::vector<Component*> components_;

  ProcessControl<true> halt_control_;
  ProcessControl<true> crash_control_;
  ByteBus data_bus_;
  WordBus address_bus_;
  ByteRegister a_;
  ByteRegister x_;
  Counter<base::Word> pc_;
  WordRegister mar_;
  StatusRegister status_;
  Controller controller_;

  mutable bool controls_indexed_ = false;
  mutable std::unordered_map<std::string, ControlBase*> controls_by_path_;
  mutable std::vector<std::string> control_paths_;
};

}  // namespace irata2::sim

#endif  // IRATA2_SIM_CPU_H
