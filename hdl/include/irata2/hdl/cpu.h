#ifndef IRATA2_HDL_CPU_H
#define IRATA2_HDL_CPU_H

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

/// Immutable HDL CPU structure. All navigation is strongly typed.
/// Path resolution has been moved to microcode::ir::CpuPathResolver.
class Cpu final : public Component<Cpu> {
 public:
  Cpu();

  const ByteBus& data_bus() const { return data_bus_; }
  const WordBus& address_bus() const { return address_bus_; }

  const ByteRegister& a() const { return a_; }
  const ByteRegister& x() const { return x_; }
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
  const Counter<base::Word> pc_;
  const StatusRegister status_;
  const Controller controller_;
  const Memory memory_;
};

/// Thread-safe singleton access to the HDL CPU.
const Cpu& GetCpu();

}  // namespace irata2::hdl

#endif  // IRATA2_HDL_CPU_H
