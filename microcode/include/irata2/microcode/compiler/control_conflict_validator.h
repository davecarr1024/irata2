#ifndef IRATA2_MICROCODE_COMPILER_CONTROL_CONFLICT_VALIDATOR_H
#define IRATA2_MICROCODE_COMPILER_CONTROL_CONFLICT_VALIDATOR_H

#include "irata2/microcode/compiler/pass.h"

namespace irata2::microcode::compiler {

/**
 * @brief Validates that controls don't conflict within a step.
 *
 * Detects mutually exclusive control combinations:
 * - Read and write on the same register/component
 * - Multiple ALU opcode bits set simultaneously
 * - Both set and clear on the same status flag
 * - Increment and decrement on the same counter
 *
 * Example valid patterns:
 * - [a.write, x.read] (different registers)
 * - [status.zero.set] (single status operation)
 * - [alu.opcode_bit_0] (single ALU opcode bit)
 *
 * Example invalid patterns:
 * - [a.read, a.write] (read+write same register)
 * - [alu.opcode_bit_0, alu.opcode_bit_1] (multiple ALU bits)
 * - [status.zero.set, status.zero.clear] (set+clear same flag)
 */
class ControlConflictValidator final : public Pass {
 public:
  void Run(ir::InstructionSet& instruction_set) const override;
};

}  // namespace irata2::microcode::compiler

#endif  // IRATA2_MICROCODE_COMPILER_CONTROL_CONFLICT_VALIDATOR_H
