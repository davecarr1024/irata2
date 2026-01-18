#ifndef IRATA2_HDL_PROGRAM_COUNTER_H
#define IRATA2_HDL_PROGRAM_COUNTER_H

#include <utility>

#include "irata2/hdl/bus.h"
#include "irata2/hdl/process_control.h"
#include "irata2/hdl/read_control.h"
#include "irata2/hdl/register.h"

namespace irata2::hdl {

class ProgramCounter final : public Register<ProgramCounter, base::Word> {
 public:
  ProgramCounter(std::string name,
                 ComponentBase& parent,
                 const Bus<base::Word>& bus,
                 const Bus<base::Byte>& data_bus)
      : Register<ProgramCounter, base::Word>(std::move(name), parent, bus),
        increment_control_("increment", *this),
        add_offset_control_("add_offset", *this, data_bus) {}

  const ProcessControl<true>& increment() const { return increment_control_; }
  const ReadControl<base::Byte>& add_offset() const { return add_offset_control_; }

  template <typename Visitor>
  void visit_impl(Visitor&& visitor) const {
    Register<ProgramCounter, base::Word>::visit_impl(visitor);
    increment_control_.visit(visitor);
    add_offset_control_.visit(visitor);
  }

 private:
  const ProcessControl<true> increment_control_;
  const ReadControl<base::Byte> add_offset_control_;
};

}  // namespace irata2::hdl

#endif  // IRATA2_HDL_PROGRAM_COUNTER_H
