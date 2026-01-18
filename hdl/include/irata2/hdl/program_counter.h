#ifndef IRATA2_HDL_PROGRAM_COUNTER_H
#define IRATA2_HDL_PROGRAM_COUNTER_H

#include <utility>

#include "irata2/hdl/bus.h"
#include "irata2/hdl/byte_register.h"
#include "irata2/hdl/process_control.h"
#include "irata2/hdl/register.h"

namespace irata2::hdl {

/// Program counter with explicit signed offset register for relative branches.
///
/// The signed_offset register holds the branch displacement read from the data
/// bus. The add_signed_offset control adds this value (interpreted as signed)
/// to the PC during the Process phase. This design keeps all state visible in
/// the HDL component tree rather than hidden in implementation.
class ProgramCounter final : public Register<ProgramCounter, base::Word> {
 public:
  ProgramCounter(std::string name,
                 ComponentBase& parent,
                 const Bus<base::Word>& bus,
                 const Bus<base::Byte>& data_bus)
      : Register<ProgramCounter, base::Word>(std::move(name), parent, bus),
        signed_offset_("signed_offset", *this, data_bus),
        increment_control_("increment", *this),
        add_signed_offset_control_("add_signed_offset", *this) {}

  const ByteRegister& signed_offset() const { return signed_offset_; }
  const ProcessControl<true>& increment() const { return increment_control_; }
  const ProcessControl<true>& add_signed_offset() const { return add_signed_offset_control_; }

  template <typename Visitor>
  void visit_impl(Visitor&& visitor) const {
    Register<ProgramCounter, base::Word>::visit_impl(visitor);
    signed_offset_.visit(visitor);
    increment_control_.visit(visitor);
    add_signed_offset_control_.visit(visitor);
  }

 private:
  const ByteRegister signed_offset_;
  const ProcessControl<true> increment_control_;
  const ProcessControl<true> add_signed_offset_control_;
};

}  // namespace irata2::hdl

#endif  // IRATA2_HDL_PROGRAM_COUNTER_H
