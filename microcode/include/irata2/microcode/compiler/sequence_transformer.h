#ifndef IRATA2_MICROCODE_COMPILER_SEQUENCE_TRANSFORMER_H
#define IRATA2_MICROCODE_COMPILER_SEQUENCE_TRANSFORMER_H

#include "irata2/hdl/control_base.h"
#include "irata2/microcode/compiler/pass.h"

namespace irata2::microcode::compiler {

class SequenceTransformer final : public Pass {
 public:
  SequenceTransformer(const hdl::ControlBase& increment_control,
                      const hdl::ControlBase& reset_control);

  void Run(ir::InstructionSet& instruction_set) const override;

 private:
  const hdl::ControlBase& increment_control_;
  const hdl::ControlBase& reset_control_;
};

}  // namespace irata2::microcode::compiler

#endif  // IRATA2_MICROCODE_COMPILER_SEQUENCE_TRANSFORMER_H
