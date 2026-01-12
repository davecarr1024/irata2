#include "irata2/microcode/compiler/empty_step_optimizer.h"

#include <vector>

namespace irata2::microcode::compiler {

void EmptyStepOptimizer::Run(ir::InstructionSet& instruction_set) const {
  // Optimize fetch preamble
  std::vector<ir::Step> non_empty_steps;
  for (const auto& step : instruction_set.fetch_preamble) {
    if (!step.controls.empty()) {
      non_empty_steps.push_back(step);
    }
  }
  instruction_set.fetch_preamble = std::move(non_empty_steps);

  // Optimize each instruction variant
  for (auto& instruction : instruction_set.instructions) {
    for (auto& variant : instruction.variants) {
      non_empty_steps.clear();
      for (const auto& step : variant.steps) {
        if (!step.controls.empty()) {
          non_empty_steps.push_back(step);
        }
      }
      variant.steps = std::move(non_empty_steps);
    }
  }
}

}  // namespace irata2::microcode::compiler
