#ifndef IRATA2_SIM_CPU_H
#define IRATA2_SIM_CPU_H

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "irata2/base/tick_phase.h"
#include "irata2/hdl/cpu.h"
#include "irata2/microcode/output/program.h"
#include "irata2/sim/alu.h"
#include "irata2/sim/byte_bus.h"
#include "irata2/sim/byte_register.h"
#include "irata2/sim/component.h"
#include "irata2/sim/control.h"
#include "irata2/sim/controller.h"
#include "irata2/sim/counter.h"
#include "irata2/sim/debug_symbols.h"
#include "irata2/sim/memory/memory.h"
#include "irata2/sim/memory/module.h"
#include "irata2/sim/memory/region.h"
#include "irata2/sim/status_register.h"
#include "irata2/sim/word_bus.h"

namespace irata2::sim {

/**
 * @brief Runtime CPU simulator with mutable state.
 *
 * The sim::Cpu class is the root of the runtime simulation, containing all
 * mutable component state and orchestrating the five-phase tick model.
 * Unlike hdl::Cpu which is an immutable schematic, this class holds actual
 * register values, bus states, and execution context.
 *
 * The CPU executes microcode programs by:
 * 1. Loading a compiled MicrocodeProgram into the Controller
 * 2. Running Tick() which executes the five-phase cycle
 * 3. Checking halted() to detect program completion
 *
 * @code
 * sim::Cpu cpu;  // Uses default HDL and microcode
 * cpu.pc().set_value(base::Word{0x8000});  // Set entry point
 * while (!cpu.halted()) {
 *   cpu.Tick();
 * }
 * @endcode
 *
 * @see hdl::Cpu for the immutable hardware schematic
 * @see base::TickPhase for the five-phase execution model
 * @see Controller for microcode ROM execution
 */
class Cpu : public Component {
 public:
  /**
   * @brief Result of running until halt.
   */
  struct RunResult {
    bool halted = false;   ///< True if CPU halted normally
    bool crashed = false;  ///< True if crash control was asserted
  };

  Cpu();
  explicit Cpu(std::shared_ptr<const hdl::Cpu> hdl,
               std::shared_ptr<const microcode::output::MicrocodeProgram> program,
               std::shared_ptr<memory::Module> cartridge_rom = nullptr,
               std::vector<memory::Region> extra_regions = {});

  Cpu& cpu() override { return *this; }
  const Cpu& cpu() const override { return *this; }

  std::string path() const override { return ""; }

  /// @brief Get current tick phase.
  base::TickPhase current_phase() const override { return current_phase_; }

  /**
   * @brief Execute one complete clock cycle (all five phases).
   *
   * Runs Control, Write, Read, Process, and Clear phases in order.
   * Does nothing if halted() is true.
   */
  void Tick();

  /**
   * @brief Run until the halt control is asserted.
   * @return RunResult indicating how execution terminated
   */
  RunResult RunUntilHalt();

  /**
   * @brief Override current phase for testing.
   * @warning For unit tests only. Allows direct control assertions.
   */
  void SetCurrentPhaseForTest(base::TickPhase phase);

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

  void LoadDebugSymbols(DebugSymbols symbols);
  const DebugSymbols* debug_symbols() const;

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
  Alu& alu() { return alu_; }
  const Alu& alu() const { return alu_; }
  Counter<base::Word>& pc() { return pc_; }
  const Counter<base::Word>& pc() const { return pc_; }
  StatusRegister& status() { return status_; }
  const StatusRegister& status() const { return status_; }
  Controller& controller() { return controller_; }
  const Controller& controller() const { return controller_; }
  memory::Memory& memory() { return memory_; }
  const memory::Memory& memory() const { return memory_; }

  ControlBase* ResolveControl(std::string_view path);
  const ControlBase* ResolveControl(std::string_view path) const;
  std::vector<std::string> AllControlPaths() const;
  const std::vector<ControlBase*>& ControlOrder() const {
    return control_order_;
  }

 void RegisterChild(Component& child) override;

 private:
  void BuildControlIndex();
  void TickPhase(void (Component::*phase)());
  void TickProcess() override;

  std::shared_ptr<const hdl::Cpu> hdl_;
  std::shared_ptr<const microcode::output::MicrocodeProgram> microcode_;
  base::TickPhase current_phase_ = base::TickPhase::None;
  bool halted_ = false;
  bool crashed_ = false;
  uint64_t cycle_count_ = 0;

  std::vector<Component*> components_;
  std::optional<DebugSymbols> debug_symbols_;

  ProcessControl<true> halt_control_;
  ProcessControl<true> crash_control_;
  ByteBus data_bus_;
  WordBus address_bus_;
  ByteRegister a_;
  ByteRegister x_;
  Counter<base::Word> pc_;
  StatusRegister status_;
  Alu alu_;
  Controller controller_;
  memory::Memory memory_;

  std::unordered_map<std::string, ControlBase*> controls_by_path_;
  std::vector<std::string> control_paths_;
  std::vector<ControlBase*> control_order_;
};

}  // namespace irata2::sim

#endif  // IRATA2_SIM_CPU_H
