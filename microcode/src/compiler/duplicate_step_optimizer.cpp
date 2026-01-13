#include "irata2/microcode/compiler/duplicate_step_optimizer.h"

#include <vector>

namespace irata2::microcode::compiler {

namespace {

bool StepsAreIdentical(const ir::Step& a, const ir::Step& b) {
  if (a.stage != b.stage) {
    return false;
  }
  if (a.controls.size() != b.controls.size()) {
    return false;
  }
  for (size_t i = 0; i < a.controls.size(); ++i) {
    if (a.controls[i] != b.controls[i]) {
      return false;
    }
  }
  return true;
}

void OptimizeSteps(std::vector<ir::Step>& steps) {
  if (steps.empty()) {
    return;
  }

  std::vector<ir::Step> deduplicated;
  deduplicated.reserve(steps.size());
  deduplicated.push_back(steps[0]);

  for (size_t i = 1; i < steps.size(); ++i) {
    if (!StepsAreIdentical(deduplicated.back(), steps[i])) {
      deduplicated.push_back(steps[i]);
    }
  }

  steps = std::move(deduplicated);
}

}  // namespace

void DuplicateStepOptimizer::Run(ir::InstructionSet& instruction_set) const {
  // Optimize fetch preamble
  OptimizeSteps(instruction_set.fetch_preamble);

  // Optimize each instruction variant
  for (auto& instruction : instruction_set.instructions) {
    for (auto& variant : instruction.variants) {
      OptimizeSteps(variant.steps);
    }
  }
}

}  // namespace irata2::microcode::compiler
