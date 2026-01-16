#ifndef IRATA2_MICROCODE_COMPILER_BUS_VALIDATOR_H
#define IRATA2_MICROCODE_COMPILER_BUS_VALIDATOR_H

#include "irata2/microcode/compiler/pass.h"

namespace irata2::microcode::compiler {

/**
 * @brief Validates bus usage patterns in microcode.
 *
 * Enforces bus discipline for each step:
 * - At most one writer per bus per step
 * - Readers on a bus require a corresponding writer in the same step
 * - Writers on a bus should have at least one reader (warning, not error)
 *
 * Bus assignments (from hdl::Cpu construction):
 * - Data bus (byte): a, x, alu, status, controller, memory
 * - Address bus (word): pc, tmp, memory
 */
class BusValidator final : public Pass {
 public:
  void Run(ir::InstructionSet& instruction_set) const override;
};

}  // namespace irata2::microcode::compiler

#endif  // IRATA2_MICROCODE_COMPILER_BUS_VALIDATOR_H
