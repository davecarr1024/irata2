#ifndef IRATA2_MICROCODE_COMPILER_PHASE_ORDERING_VALIDATOR_H
#define IRATA2_MICROCODE_COMPILER_PHASE_ORDERING_VALIDATOR_H

#include "irata2/microcode/compiler/pass.h"

namespace irata2::microcode::compiler {

/**
 * @brief Validates phase ordering in microcode steps.
 *
 * Enforces the five-phase tick model:
 * 1. Control - controller reads microcode
 * 2. Write - components write to buses
 * 3. Read - components read from buses
 * 4. Process - internal updates (ALU, status flags)
 * 5. Clear - reset auto-clear controls
 *
 * Rules:
 * - Write controls must come before Read controls in the phase order
 * - Process controls must come after Read controls
 * - Controls can only be asserted during their designated phase
 *
 * Example valid patterns:
 * - [a.write, x.read] (Write phase 2 < Read phase 3)
 * - [pc.write, memory.mar.read, memory.write, a.read] (phases 2,3,2,3)
 *
 * Example invalid patterns:
 * - [a.read, x.write] if they're on the same bus (Read phase 3 > Write phase 2)
 */
class PhaseOrderingValidator final : public Pass {
 public:
  void Run(ir::InstructionSet& instruction_set) const override;
};

}  // namespace irata2::microcode::compiler

#endif  // IRATA2_MICROCODE_COMPILER_PHASE_ORDERING_VALIDATOR_H
