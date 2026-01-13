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

namespace irata2::microcode::compiler {

class Compiler {
 public:
  Compiler(encoder::ControlEncoder control_encoder,
           encoder::StatusEncoder status_encoder,
           const hdl::ControlInfo& increment_control,
           const hdl::ControlInfo& reset_control);

  output::MicrocodeProgram Compile(ir::InstructionSet instruction_set) const;

 private:
  encoder::ControlEncoder control_encoder_;
  encoder::StatusEncoder status_encoder_;
  FetchTransformer fetch_transformer_;
  FetchValidator fetch_validator_;
  BusValidator bus_validator_;
  ControlConflictValidator control_conflict_validator_;
  StageValidator stage_validator_;
  StatusValidator status_validator_;
  IsaCoverageValidator isa_coverage_validator_;
  SequenceTransformer sequence_transformer_;
  SequenceValidator sequence_validator_;
  EmptyStepOptimizer empty_step_optimizer_;
  DuplicateStepOptimizer duplicate_step_optimizer_;
  StepMergingOptimizer step_merging_optimizer_;
};

}  // namespace irata2::microcode::compiler

#endif  // IRATA2_MICROCODE_COMPILER_COMPILER_H
