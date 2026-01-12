#ifndef IRATA2_MICROCODE_COMPILER_STAGE_VALIDATOR_H
#define IRATA2_MICROCODE_COMPILER_STAGE_VALIDATOR_H

#include "irata2/microcode/compiler/pass.h"

namespace irata2::microcode::compiler {

/**
 * @brief Validates stage numbering in microcode.
 *
 * Enforces stage discipline:
 * - Stages start at 0
 * - Stages are monotonically increasing
 * - No gaps in stage sequence
 * - Steps within a stage are ordered by appearance
 *
 * Example valid patterns:
 * - Steps with stages [0, 0, 0, 1, 1, 2]
 * - Steps with stages [0, 1, 2, 3]
 * - Steps with stages [0] (single stage)
 *
 * Example invalid patterns:
 * - Steps with stages [1, 2, 3] (doesn't start at 0)
 * - Steps with stages [0, 0, 2, 2] (gap: missing stage 1)
 * - Steps with stages [0, 1, 0, 1] (not monotonic)
 */
class StageValidator final : public Pass {
 public:
  void Run(ir::InstructionSet& instruction_set) const override;
};

}  // namespace irata2::microcode::compiler

#endif  // IRATA2_MICROCODE_COMPILER_STAGE_VALIDATOR_H
