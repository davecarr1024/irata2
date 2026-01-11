#ifndef IRATA2_HDL_CONTROLLER_H
#define IRATA2_HDL_CONTROLLER_H

#include "irata2/hdl/byte_bus.h"
#include "irata2/hdl/byte_counter.h"
#include "irata2/hdl/byte_register.h"
#include "irata2/hdl/component_with_parent.h"
#include "irata2/hdl/latched_word_register.h"
#include "irata2/hdl/process_control.h"

#include <utility>

namespace irata2::hdl {

class Controller final : public ComponentWithParent<Controller> {
 public:
  Controller(std::string name, ComponentBase& parent, const ByteBus& data_bus);

  const ByteRegister& ir() const { return ir_; }
  const ByteCounter& sc() const { return sc_; }
  const LatchedWordRegister& ipc() const { return ipc_; }
  const ProcessControl<true>& ipc_latch() const { return ipc_latch_; }

  template <typename Visitor>
  void visit_impl(Visitor&& visitor) const {
    visitor(*this);
    ir_.visit(visitor);
    sc_.visit(visitor);
    ipc_.visit(visitor);
    ipc_latch_.visit(visitor);
  }

 private:
  const ByteRegister ir_;
  const ByteCounter sc_;
  const LatchedWordRegister ipc_;
  const ProcessControl<true> ipc_latch_;
};

}  // namespace irata2::hdl

#endif  // IRATA2_HDL_CONTROLLER_H
