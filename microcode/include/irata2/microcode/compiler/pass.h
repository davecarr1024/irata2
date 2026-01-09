#ifndef IRATA2_MICROCODE_COMPILER_PASS_H
#define IRATA2_MICROCODE_COMPILER_PASS_H

#include "irata2/microcode/ir/instruction_set.h"

namespace irata2::microcode::compiler {

class Pass {
 public:
  virtual ~Pass() = default;
  virtual void Run(ir::InstructionSet& instruction_set) const = 0;
};

}  // namespace irata2::microcode::compiler

#endif  // IRATA2_MICROCODE_COMPILER_PASS_H
