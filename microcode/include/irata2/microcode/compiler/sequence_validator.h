#ifndef IRATA2_MICROCODE_COMPILER_SEQUENCE_VALIDATOR_H
#define IRATA2_MICROCODE_COMPILER_SEQUENCE_VALIDATOR_H

#include "irata2/hdl/control_info.h"
#include "irata2/microcode/compiler/pass.h"

namespace irata2::microcode::compiler {

class SequenceValidator final : public Pass {
 public:
  SequenceValidator(const hdl::ControlInfo& increment_control,
                    const hdl::ControlInfo& reset_control);

  void Run(ir::InstructionSet& instruction_set) const override;

 private:
  const hdl::ControlInfo& increment_control_;
  const hdl::ControlInfo& reset_control_;
};

}  // namespace irata2::microcode::compiler

#endif  // IRATA2_MICROCODE_COMPILER_SEQUENCE_VALIDATOR_H
