#ifndef IRATA2_HDL_CPU_H
#define IRATA2_HDL_CPU_H

#include "irata2/hdl/alu.h"
#include "irata2/hdl/byte_bus.h"
#include "irata2/hdl/byte_register.h"
#include "irata2/hdl/component.h"
#include "irata2/hdl/controller.h"
#include "irata2/hdl/counter.h"
#include "irata2/hdl/memory.h"
#include "irata2/hdl/process_control.h"
#include "irata2/hdl/status_register.h"
#include "irata2/hdl/word_bus.h"
#include "irata2/hdl/word_register.h"

namespace irata2::hdl {

/**
 * @brief Immutable CPU hardware structure definition (schematic).
 *
 * The Cpu class represents the complete hardware schematic as an immutable
 * tree of components. All members are const and initialized at construction,
 * making the HDL thread-safe and suitable for sharing across compilation units.
 *
 * This class contains no runtime state - it purely describes the hardware
 * structure. For simulation with runtime state, see sim::Cpu.
 *
 * Navigation is strongly typed via accessor methods. String-based path
 * resolution for microcode compilation lives in microcode::ir::CpuPathResolver.
 *
 * @code
 * const auto& cpu = hdl::GetCpu();
 * const auto& a_reg = cpu.a();           // Strongly typed
 * const auto& write_ctrl = a_reg.write(); // Navigate to controls
 * @endcode
 *
 * @see GetCpu() for thread-safe singleton access
 * @see sim::Cpu for runtime simulator with mutable state
 * @see microcode::ir::CpuPathResolver for string path resolution
 */
class Cpu final : public Component<Cpu> {
 public:
  Cpu();

  const ByteBus& data_bus() const { return data_bus_; }
  const WordBus& address_bus() const { return address_bus_; }

  const ByteRegister& a() const { return a_; }
  const ByteRegister& x() const { return x_; }
  const Alu& alu() const { return alu_; }
  const Counter<base::Word>& pc() const { return pc_; }
  const StatusRegister& status() const { return status_; }
  const Controller& controller() const { return controller_; }
  const Memory& memory() const { return memory_; }
  const ProcessControl<true>& halt() const { return halt_control_; }
  const ProcessControl<true>& crash() const { return crash_control_; }

  template <typename Visitor>
  void visit_impl(Visitor&& visitor) const {
    visitor(*this);
    halt_control_.visit(visitor);
    crash_control_.visit(visitor);
    data_bus_.visit(visitor);
    address_bus_.visit(visitor);
    a_.visit(visitor);
    x_.visit(visitor);
    alu_.visit(visitor);
    pc_.visit(visitor);
    status_.visit(visitor);
    controller_.visit(visitor);
    memory_.visit(visitor);
  }

 private:
  const ProcessControl<true> halt_control_;
  const ProcessControl<true> crash_control_;
  const ByteBus data_bus_;
  const WordBus address_bus_;
  const ByteRegister a_;
  const ByteRegister x_;
  const Alu alu_;
  const Counter<base::Word> pc_;
  const StatusRegister status_;
  const Controller controller_;
  const Memory memory_;
};

/**
 * @brief Thread-safe singleton access to the HDL CPU.
 *
 * Returns a reference to the single immutable Cpu instance. Uses C++11
 * static initialization guarantees for thread safety. The instance is
 * created on first call and persists for program lifetime.
 *
 * @return Reference to the global immutable HDL Cpu
 */
const Cpu& GetCpu();

}  // namespace irata2::hdl

#endif  // IRATA2_HDL_CPU_H
