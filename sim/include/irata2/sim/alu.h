#ifndef IRATA2_SIM_ALU_H
#define IRATA2_SIM_ALU_H

#include "irata2/base/types.h"
#include "irata2/sim/byte_register.h"
#include "irata2/sim/component.h"
#include "irata2/sim/control.h"
#include "irata2/sim/status.h"
#include "irata2/sim/status_register.h"

namespace irata2::sim {

class Alu final : public ComponentWithParent {
 public:
  Alu(std::string name,
      Component& parent,
      Bus<base::Byte>& data_bus,
      StatusRegister& status);

  ByteRegister& lhs() { return lhs_; }
  const ByteRegister& lhs() const { return lhs_; }
  ByteRegister& rhs() { return rhs_; }
  const ByteRegister& rhs() const { return rhs_; }
  ByteRegister& result() { return result_; }
  const ByteRegister& result() const { return result_; }

  ProcessControl<true>& opcode_bit_0() { return opcode_bit_0_; }
  const ProcessControl<true>& opcode_bit_0() const { return opcode_bit_0_; }
  ProcessControl<true>& opcode_bit_1() { return opcode_bit_1_; }
  const ProcessControl<true>& opcode_bit_1() const { return opcode_bit_1_; }
  ProcessControl<true>& opcode_bit_2() { return opcode_bit_2_; }
  const ProcessControl<true>& opcode_bit_2() const { return opcode_bit_2_; }
  ProcessControl<true>& opcode_bit_3() { return opcode_bit_3_; }
  const ProcessControl<true>& opcode_bit_3() const { return opcode_bit_3_; }

  void TickProcess() override;

 private:
  bool CarryIn() const;
  void SetCarryOut(bool value);

  ByteRegister lhs_;
  ByteRegister rhs_;
  ByteRegister result_;
  ProcessControl<true> opcode_bit_0_;
  ProcessControl<true> opcode_bit_1_;
  ProcessControl<true> opcode_bit_2_;
  ProcessControl<true> opcode_bit_3_;
  Status& carry_;
  Status& overflow_;
};

}  // namespace irata2::sim

#endif  // IRATA2_SIM_ALU_H
