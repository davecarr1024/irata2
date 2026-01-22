#ifndef IRATA2_MICROCODE_COMPILER_COMPILER_H
#define IRATA2_MICROCODE_COMPILER_COMPILER_H

#include "irata2/hdl/control_info.h"
#include "irata2/microcode/compiler/bus_validator.h"
#include "irata2/microcode/compiler/control_conflict_validator.h"
#include "irata2/microcode/compiler/duplicate_step_optimizer.h"
#include "irata2/microcode/compiler/empty_step_optimizer.h"
#include "irata2/microcode/compiler/fetch_transformer.h"
#include "irata2/microcode/compiler/fetch_validator.h"
#include "irata2/microcode/compiler/isa_coverage_validator.h"
#include "irata2/microcode/compiler/sequence_transformer.h"
#include "irata2/microcode/compiler/sequence_validator.h"
#include "irata2/microcode/compiler/stage_validator.h"
#include "irata2/microcode/compiler/status_validator.h"
#include "irata2/microcode/compiler/step_merging_optimizer.h"
#include "irata2/microcode/encoder/control_encoder.h"
#include "irata2/microcode/encoder/instruction_encoder.h"
#include "irata2/microcode/encoder/status_encoder.h"
#include "irata2/microcode/output/program.h"

namespace irata2::hdl {
class Cpu;
}  // namespace irata2::hdl

namespace irata2::microcode::compiler {

/**
 * @brief Compiles microcode IR into a MicrocodeProgram.
 *
 * The compiler runs passes in a structured order:
 *
 * 1. **Preamble passes** - Required to get microcode to valid state
 *    - FetchTransformer: Adds fetch preamble steps
 *    - FetchValidator: Validates fetch preamble structure
 *    - SequenceTransformer: Adds step increment/reset controls
 *
 * 2. **Initial validation** - All validators run to verify preamble output
 *
 * 3. **Optimization passes** - Each optimizer followed by all validators
 *    - EmptyStepOptimizer -> validators
 *    - DuplicateStepOptimizer -> validators
 *    - StepMergingOptimizer -> validators
 *
 * 4. **Encoding** - Convert validated IR to output format
 *
 * This defensive structure ensures that any pass producing invalid output
 * is immediately detected by the validators.
 */
class Compiler {
 public:
  Compiler(encoder::ControlEncoder control_encoder,
           encoder::StatusEncoder status_encoder,
           const hdl::Cpu& cpu,
           const hdl::ControlInfo& increment_control,
           const hdl::ControlInfo& reset_control);

  output::MicrocodeProgram Compile(ir::InstructionSet instruction_set) const;

 private:
  // Run all validators on the instruction set
  void RunAllValidators(ir::InstructionSet& instruction_set) const;

  // Encode the validated IR into the output program
  output::MicrocodeProgram Encode(const ir::InstructionSet& instruction_set) const;

  encoder::ControlEncoder control_encoder_;
  encoder::StatusEncoder status_encoder_;

  // Preamble passes
  FetchTransformer fetch_transformer_;
  FetchValidator fetch_validator_;
  SequenceTransformer sequence_transformer_;

  // Validators
  BusValidator bus_validator_;
  ControlConflictValidator control_conflict_validator_;
  StageValidator stage_validator_;
  StatusValidator status_validator_;
  IsaCoverageValidator isa_coverage_validator_;
  SequenceValidator sequence_validator_;

  // Optimizers
  EmptyStepOptimizer empty_step_optimizer_;
  DuplicateStepOptimizer duplicate_step_optimizer_;
  StepMergingOptimizer step_merging_optimizer_;
};

}  // namespace irata2::microcode::compiler

#endif  // IRATA2_MICROCODE_COMPILER_COMPILER_H
