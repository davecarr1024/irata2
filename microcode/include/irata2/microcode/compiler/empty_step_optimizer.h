#ifndef IRATA2_MICROCODE_COMPILER_EMPTY_STEP_OPTIMIZER_H
#define IRATA2_MICROCODE_COMPILER_EMPTY_STEP_OPTIMIZER_H

#include "irata2/microcode/compiler/pass.h"

namespace irata2::microcode::compiler {

/**
 * @brief Removes steps with no asserted controls.
 *
 * Empty steps (steps with zero controls) add no behavior but increase
 * step count and ROM usage. This optimizer removes them while preserving
 * stage boundaries.
 *
 * Example transformation:
 * Before:
 *   stage 0: step 0 [a.read, b.write]
 *   stage 0: step 1 []              <- empty, removed
 *   stage 1: step 2 [c.read]
 *
 * After:
 *   stage 0: step 0 [a.read, b.write]
 *   stage 1: step 1 [c.read]         <- renumbered
 *
 * Stage boundaries are preserved - empty steps at stage boundaries do not
 * cause stages to merge.
 */
class EmptyStepOptimizer final : public Pass {
 public:
  void Run(ir::InstructionSet& instruction_set) const override;
};

}  // namespace irata2::microcode::compiler

#endif  // IRATA2_MICROCODE_COMPILER_EMPTY_STEP_OPTIMIZER_H
