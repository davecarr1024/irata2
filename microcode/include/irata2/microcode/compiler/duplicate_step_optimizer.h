#ifndef IRATA2_MICROCODE_COMPILER_DUPLICATE_STEP_OPTIMIZER_H
#define IRATA2_MICROCODE_COMPILER_DUPLICATE_STEP_OPTIMIZER_H

#include "irata2/microcode/compiler/pass.h"

namespace irata2::microcode::compiler {

/**
 * @brief Collapses adjacent steps with identical control sets.
 *
 * Consecutive steps within the same stage that assert the exact same controls
 * provide no additional behavior and waste ROM. This optimizer merges them
 * into a single step.
 *
 * Example transformation:
 * Before:
 *   stage 0: step 0 [a.read, b.write]
 *   stage 0: step 1 [a.read, b.write]  <- duplicate, removed
 *   stage 1: step 2 [c.read]
 *
 * After:
 *   stage 0: step 0 [a.read, b.write]
 *   stage 1: step 1 [c.read]           <- renumbered
 *
 * Only adjacent steps are merged - non-consecutive identical steps are
 * preserved as they may have different timing requirements.
 *
 * Stage boundaries prevent merging - steps at different stages are never
 * merged even if their controls match.
 */
class DuplicateStepOptimizer final : public Pass {
 public:
  void Run(ir::InstructionSet& instruction_set) const override;
};

}  // namespace irata2::microcode::compiler

#endif  // IRATA2_MICROCODE_COMPILER_DUPLICATE_STEP_OPTIMIZER_H
