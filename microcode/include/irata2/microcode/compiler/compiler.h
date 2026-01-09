#ifndef IRATA2_MICROCODE_COMPILER_COMPILER_H
#define IRATA2_MICROCODE_COMPILER_COMPILER_H

#include "irata2/microcode/compiler/fetch_transformer.h"
#include "irata2/microcode/compiler/fetch_validator.h"
#include "irata2/microcode/compiler/isa_coverage_validator.h"
#include "irata2/microcode/compiler/sequence_transformer.h"
#include "irata2/microcode/compiler/sequence_validator.h"
#include "irata2/microcode/encoder/control_encoder.h"
#include "irata2/microcode/encoder/instruction_encoder.h"
#include "irata2/microcode/encoder/status_encoder.h"
#include "irata2/microcode/output/program.h"

namespace irata2::microcode::compiler {

class Compiler {
 public:
  Compiler(encoder::ControlEncoder control_encoder,
           encoder::StatusEncoder status_encoder,
           const hdl::ControlBase& increment_control,
           const hdl::ControlBase& reset_control);

  output::MicrocodeProgram Compile(ir::InstructionSet instruction_set) const;

 private:
  encoder::ControlEncoder control_encoder_;
  encoder::StatusEncoder status_encoder_;
  FetchTransformer fetch_transformer_;
  FetchValidator fetch_validator_;
  IsaCoverageValidator isa_coverage_validator_;
  SequenceTransformer sequence_transformer_;
  SequenceValidator sequence_validator_;
};

}  // namespace irata2::microcode::compiler

#endif  // IRATA2_MICROCODE_COMPILER_COMPILER_H
