#include "irata2/microcode/compiler/compiler.h"

#include "irata2/microcode/error.h"

#include <sstream>

namespace irata2::microcode::compiler {

Compiler::Compiler(encoder::ControlEncoder control_encoder,
                   encoder::StatusEncoder status_encoder,
                   const hdl::Cpu& cpu,
                   const hdl::ControlInfo& increment_control,
                   const hdl::ControlInfo& reset_control)
    : control_encoder_(std::move(control_encoder)),
      status_encoder_(std::move(status_encoder)),
      bus_validator_(cpu),
      status_validator_(status_encoder_),
      sequence_transformer_(increment_control, reset_control),
      sequence_validator_(increment_control, reset_control) {}

void Compiler::RunAllValidators(ir::InstructionSet& instruction_set) const {
  bus_validator_.Run(instruction_set);
  control_conflict_validator_.Run(instruction_set);
  stage_validator_.Run(instruction_set);
  status_validator_.Run(instruction_set);
  isa_coverage_validator_.Run(instruction_set);
  sequence_validator_.Run(instruction_set);
}

output::MicrocodeProgram Compiler::Encode(const ir::InstructionSet& instruction_set) const {
  output::MicrocodeProgram program;
  program.control_paths = control_encoder_.control_paths();
  program.status_bits = status_encoder_.bits();

  for (const auto& instruction : instruction_set.instructions) {
    const auto opcode_value = static_cast<uint32_t>(instruction.opcode);
    if (opcode_value > 0xFF) {
      std::ostringstream message;
      message << "opcode out of range for instruction memory: " << opcode_value;
      throw microcode::MicrocodeError(message.str());
    }
    for (const auto& variant : instruction.variants) {
      const auto statuses = status_encoder_.ExpandPartial(variant.status_conditions);
      for (size_t step_index = 0; step_index < variant.steps.size(); ++step_index) {
        if (step_index > 0xFF) {
          std::ostringstream message;
          message << "step index out of range for instruction memory: "
                  << step_index;
          throw microcode::MicrocodeError(message.str());
        }
        const auto& step = variant.steps[step_index];
        const auto control_word = control_encoder_.Encode(step.controls);
        for (const auto status : statuses) {
          const auto key = encoder::InstructionEncoder::MakeKey(
              instruction.opcode,
              static_cast<uint8_t>(step_index),
              status);
          const uint32_t encoded_key = output::EncodeKey(key);
          auto [it, inserted] = program.table.emplace(encoded_key, control_word);
          if (!inserted && it->second != control_word) {
            std::ostringstream message;
            message << "microcode conflict at opcode "
                    << static_cast<int>(key.opcode)
                    << " step " << static_cast<int>(key.step)
                    << " status " << static_cast<int>(key.status);
            throw microcode::MicrocodeError(message.str());
          }
        }
      }
    }
  }

  return program;
}

output::MicrocodeProgram Compiler::Compile(ir::InstructionSet instruction_set) const {
  // Phase 1: Preamble passes - required to get microcode to valid state
  fetch_transformer_.Run(instruction_set);
  fetch_validator_.Run(instruction_set);
  sequence_transformer_.Run(instruction_set);

  // Phase 2: Initial validation - verify preamble output is valid
  RunAllValidators(instruction_set);

  // Phase 3: Optimization passes - each optimizer followed by all validators
  empty_step_optimizer_.Run(instruction_set);
  RunAllValidators(instruction_set);

  duplicate_step_optimizer_.Run(instruction_set);
  RunAllValidators(instruction_set);

  step_merging_optimizer_.Run(instruction_set);
  RunAllValidators(instruction_set);

  // Phase 4: Encoding - convert validated IR to output format
  return Encode(instruction_set);
}

}  // namespace irata2::microcode::compiler
