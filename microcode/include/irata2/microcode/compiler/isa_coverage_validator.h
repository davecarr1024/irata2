#ifndef IRATA2_MICROCODE_COMPILER_ISA_COVERAGE_VALIDATOR_H
#define IRATA2_MICROCODE_COMPILER_ISA_COVERAGE_VALIDATOR_H

#include "irata2/microcode/compiler/pass.h"

namespace irata2::microcode::compiler {

class IsaCoverageValidator final : public Pass {
 public:
  void Run(ir::InstructionSet& instruction_set) const override;
};

}  // namespace irata2::microcode::compiler

#endif  // IRATA2_MICROCODE_COMPILER_ISA_COVERAGE_VALIDATOR_H
