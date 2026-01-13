#ifndef IRATA2_MICROCODE_COMPILER_STATUS_VALIDATOR_H
#define IRATA2_MICROCODE_COMPILER_STATUS_VALIDATOR_H

#include "irata2/microcode/compiler/pass.h"
#include "irata2/microcode/encoder/status_encoder.h"

namespace irata2::microcode::compiler {

/**
 * @brief Validates status variant coverage in microcode.
 *
 * Enforces status discipline for conditional instructions:
 * - Each variant specifies at most one status bit (single-status rule)
 * - All variants together cover exactly all possible status combinations
 * - No overlaps or gaps in status coverage
 * - If any variant specifies a status, the opposite variant must exist
 *
 * Example valid patterns:
 * - No variants (unconditional instruction like HLT)
 * - Single variant with no status (unconditional like NOP)
 * - Two variants: {zero: true} and {zero: false} (conditional like JEQ)
 *
 * Example invalid patterns:
 * - Variant with {zero: true, carry: true} (multi-status)
 * - Only {zero: true} without {zero: false} (incomplete coverage)
 * - Both {zero: true} and {zero: true, carry: false} (overlap)
 */
class StatusValidator final : public Pass {
 public:
  explicit StatusValidator(const encoder::StatusEncoder& status_encoder);

  void Run(ir::InstructionSet& instruction_set) const override;

 private:
  const encoder::StatusEncoder& status_encoder_;
};

}  // namespace irata2::microcode::compiler

#endif  // IRATA2_MICROCODE_COMPILER_STATUS_VALIDATOR_H
