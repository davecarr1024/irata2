#include "irata2/microcode/compiler/stage_validator.h"

#include "irata2/microcode/error.h"

#include <set>
#include <sstream>

namespace irata2::microcode::compiler {

void StageValidator::Run(ir::InstructionSet& instruction_set) const {
  // Validate fetch preamble
  if (!instruction_set.fetch_preamble.empty()) {
    int prev_stage = -1;
    for (size_t i = 0; i < instruction_set.fetch_preamble.size(); ++i) {
      const auto& step = instruction_set.fetch_preamble[i];

      if (i == 0 && step.stage != 0) {
        std::ostringstream message;
        message << "fetch preamble must start at stage 0, got stage "
                << step.stage;
        throw MicrocodeError(message.str());
      }

      if (step.stage < prev_stage) {
        std::ostringstream message;
        message << "fetch preamble stages not monotonic at step " << i
                << ": stage " << step.stage << " after stage " << prev_stage;
        throw MicrocodeError(message.str());
      }

      prev_stage = step.stage;
    }

    // Check for gaps
    std::set<int> stages;
    for (const auto& step : instruction_set.fetch_preamble) {
      stages.insert(step.stage);
    }

    int expected_stage = 0;
    for (int stage : stages) {
      if (stage != expected_stage) {
        std::ostringstream message;
        message << "fetch preamble has gap in stages: expected stage "
                << expected_stage << ", found stage " << stage;
        throw MicrocodeError(message.str());
      }
      ++expected_stage;
    }
  }

  // Validate each instruction variant
  for (const auto& instruction : instruction_set.instructions) {
    for (const auto& variant : instruction.variants) {
      if (variant.steps.empty()) {
        continue;
      }

      // Check that first step starts at stage 0
      if (variant.steps[0].stage != 0) {
        std::ostringstream message;
        message << "instruction variant in opcode "
                << static_cast<int>(instruction.opcode)
                << " must start at stage 0, got stage "
                << variant.steps[0].stage;
        throw MicrocodeError(message.str());
      }

      // Check monotonic ordering
      int prev_stage = -1;
      for (size_t i = 0; i < variant.steps.size(); ++i) {
        const auto& step = variant.steps[i];

        if (step.stage < prev_stage) {
          std::ostringstream message;
          message << "opcode " << static_cast<int>(instruction.opcode)
                  << " stages not monotonic at step " << i
                  << ": stage " << step.stage << " after stage " << prev_stage;
          throw MicrocodeError(message.str());
        }

        prev_stage = step.stage;
      }

      // Check for gaps
      std::set<int> stages;
      for (const auto& step : variant.steps) {
        stages.insert(step.stage);
      }

      int expected_stage = 0;
      for (int stage : stages) {
        if (stage != expected_stage) {
          std::ostringstream message;
          message << "opcode " << static_cast<int>(instruction.opcode)
                  << " has gap in stages: expected stage "
                  << expected_stage << ", found stage " << stage;
          throw MicrocodeError(message.str());
        }
        ++expected_stage;
      }
    }
  }
}

}  // namespace irata2::microcode::compiler
