#ifndef IRATA2_HDL_ALU_H
#define IRATA2_HDL_ALU_H

#include "irata2/hdl/byte_bus.h"
#include "irata2/hdl/byte_register.h"
#include "irata2/hdl/component_with_parent.h"
#include "irata2/hdl/process_control.h"

#include <utility>

namespace irata2::hdl {

class Alu final : public ComponentWithParent<Alu> {
 public:
  Alu(std::string name, ComponentBase& parent, const ByteBus& data_bus)
      : ComponentWithParent<Alu>(std::move(name), parent),
        lhs_("lhs", *this, data_bus),
        rhs_("rhs", *this, data_bus),
        result_("result", *this, data_bus),
        opcode_bit_0_("opcode_bit_0", *this),
        opcode_bit_1_("opcode_bit_1", *this),
        opcode_bit_2_("opcode_bit_2", *this),
        opcode_bit_3_("opcode_bit_3", *this) {}

  const ByteRegister& lhs() const { return lhs_; }
  const ByteRegister& rhs() const { return rhs_; }
  const ByteRegister& result() const { return result_; }

  const ProcessControl<true>& opcode_bit_0() const { return opcode_bit_0_; }
  const ProcessControl<true>& opcode_bit_1() const { return opcode_bit_1_; }
  const ProcessControl<true>& opcode_bit_2() const { return opcode_bit_2_; }
  const ProcessControl<true>& opcode_bit_3() const { return opcode_bit_3_; }

  template <typename Visitor>
  void visit_impl(Visitor&& visitor) const {
    visitor(*this);
    lhs_.visit(visitor);
    rhs_.visit(visitor);
    result_.visit(visitor);
    opcode_bit_0_.visit(visitor);
    opcode_bit_1_.visit(visitor);
    opcode_bit_2_.visit(visitor);
    opcode_bit_3_.visit(visitor);
  }

 private:
  const ByteRegister lhs_;
  const ByteRegister rhs_;
  const ByteRegister result_;
  const ProcessControl<true> opcode_bit_0_;
  const ProcessControl<true> opcode_bit_1_;
  const ProcessControl<true> opcode_bit_2_;
  const ProcessControl<true> opcode_bit_3_;
};

}  // namespace irata2::hdl

#endif  // IRATA2_HDL_ALU_H
