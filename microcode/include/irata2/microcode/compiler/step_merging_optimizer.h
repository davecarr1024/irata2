#ifndef IRATA2_MICROCODE_COMPILER_STEP_MERGING_OPTIMIZER_H
#define IRATA2_MICROCODE_COMPILER_STEP_MERGING_OPTIMIZER_H

#include "irata2/microcode/compiler/pass.h"

namespace irata2::microcode::compiler {

/**
 * @brief Merges adjacent steps when phase ordering allows.
 *
 * Within a single tick, controls execute in phase order:
 * Control < Write < Read < Process < Clear
 *
 * Two adjacent steps in the same stage can be merged if all controls in
 * the first step have phases strictly less than all controls in the second
 * step. This ensures their combined controls execute in the correct order.
 *
 * We use strict ordering (< not <=) because merging same-phase controls
 * can change semantics - e.g., two Process phase controls in the same tick
 * behave differently than in consecutive ticks.
 *
 * Phase ordering rules:
 * - Control a < control b if a.phase < b.phase
 * - Step a < step b if max_phase(a) < min_phase(b)
 * - Steps are mergeable if a < b AND same stage
 *
 * Example transformation:
 * Before:
 *   stage 0: step 0 [pc.write]          (Write phase)
 *   stage 0: step 1 [mar.read]          (Read phase)
 *   stage 0: step 2 [memory.write]      (Write phase)
 *
 * After:
 *   stage 0: step 0 [pc.write, mar.read] (merged: Write < Read)
 *   stage 0: step 1 [memory.write]       (not merged: Read > Write)
 *
 * This optimization reduces the number of microcode steps, saving ROM space
 * and improving execution speed.
 */
class StepMergingOptimizer final : public Pass {
 public:
  void Run(ir::InstructionSet& instruction_set) const override;
};

}  // namespace irata2::microcode::compiler

#endif  // IRATA2_MICROCODE_COMPILER_STEP_MERGING_OPTIMIZER_H
