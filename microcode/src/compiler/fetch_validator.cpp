#include "irata2/microcode/compiler/fetch_validator.h"

#include "irata2/microcode/error.h"

#include <sstream>

namespace irata2::microcode::compiler {

namespace {
bool ControlsEqual(const ir::Step& left, const ir::Step& right) {
  if (left.controls.size() != right.controls.size()) {
    return false;
  }
  for (size_t i = 0; i < left.controls.size(); ++i) {
    if (left.controls[i] != right.controls[i]) {
      return false;
    }
  }
  return true;
}
}  // namespace

void FetchValidator::Run(ir::InstructionSet& instruction_set) const {
  if (instruction_set.fetch_preamble.empty()) {
    return;
  }

  for (const auto& instruction : instruction_set.instructions) {
    for (const auto& variant : instruction.variants) {
      std::vector<ir::Step> stage_zero;
      for (const auto& step : variant.steps) {
        if (step.stage == 0) {
          stage_zero.push_back(step);
        }
      }

      if (stage_zero.size() != instruction_set.fetch_preamble.size()) {
        std::ostringstream message;
        message << "fetch preamble mismatch for opcode "
                << static_cast<int>(instruction.opcode);
        throw MicrocodeError(message.str());
      }

      for (size_t i = 0; i < stage_zero.size(); ++i) {
        if (!ControlsEqual(stage_zero[i], instruction_set.fetch_preamble[i])) {
          std::ostringstream message;
          message << "fetch preamble mismatch for opcode "
                  << static_cast<int>(instruction.opcode)
                  << " at step " << i;
          throw MicrocodeError(message.str());
        }
      }
    }
  }
}

}  // namespace irata2::microcode::compiler
