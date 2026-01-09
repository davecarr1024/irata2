#ifndef IRATA2_MICROCODE_COMPILER_FETCH_VALIDATOR_H
#define IRATA2_MICROCODE_COMPILER_FETCH_VALIDATOR_H

#include "irata2/microcode/compiler/pass.h"

namespace irata2::microcode::compiler {

class FetchValidator final : public Pass {
 public:
  void Run(ir::InstructionSet& instruction_set) const override;
};

}  // namespace irata2::microcode::compiler

#endif  // IRATA2_MICROCODE_COMPILER_FETCH_VALIDATOR_H
