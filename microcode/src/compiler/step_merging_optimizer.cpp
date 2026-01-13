#include "irata2/microcode/compiler/step_merging_optimizer.h"

#include "irata2/base/tick_phase.h"

#include <algorithm>
#include <vector>

namespace irata2::microcode::compiler {

namespace {

// Get the maximum (latest) phase from a step's controls
base::TickPhase MaxPhase(const ir::Step& step) {
  if (step.controls.empty()) {
    return base::TickPhase::None;
  }
  base::TickPhase max = base::TickPhase::None;
  for (const auto* control : step.controls) {
    if (control->phase > max) {
      max = control->phase;
    }
  }
  return max;
}

// Get the minimum (earliest) phase from a step's controls
base::TickPhase MinPhase(const ir::Step& step) {
  if (step.controls.empty()) {
    return base::TickPhase::Clear;  // Any phase is <= Clear
  }
  base::TickPhase min = base::TickPhase::Clear;
  for (const auto* control : step.controls) {
    if (control->phase < min) {
      min = control->phase;
    }
  }
  return min;
}

// Check if step a strictly precedes step b in phase ordering.
// We use strict ordering (< not <=) because merging same-phase controls can
// change semantics - e.g., two Process phase controls in the same tick behave
// differently than in consecutive ticks.
bool StepStrictlyPrecedes(const ir::Step& a, const ir::Step& b) {
  // Empty step a can always precede b
  if (a.controls.empty()) {
    return true;
  }
  // Empty step b means a cannot precede it
  if (b.controls.empty()) {
    return false;
  }
  // a < b if max_phase(a) < min_phase(b) (strictly less than)
  return MaxPhase(a) < MinPhase(b);
}

// Check if two steps can be merged
bool CanMerge(const ir::Step& a, const ir::Step& b) {
  // Must be in the same stage
  if (a.stage != b.stage) {
    return false;
  }
  // a must strictly precede b in phase ordering
  return StepStrictlyPrecedes(a, b);
}

// Merge step b into step a (modifies a in place)
void MergeSteps(ir::Step& a, const ir::Step& b) {
  // Add all controls from b to a
  for (const auto* control : b.controls) {
    // Avoid duplicates
    if (std::find(a.controls.begin(), a.controls.end(), control) ==
        a.controls.end()) {
      a.controls.push_back(control);
    }
  }
}

void OptimizeSteps(std::vector<ir::Step>& steps) {
  if (steps.size() < 2) {
    return;
  }

  std::vector<ir::Step> optimized;
  optimized.reserve(steps.size());
  optimized.push_back(steps[0]);

  for (size_t i = 1; i < steps.size(); ++i) {
    if (CanMerge(optimized.back(), steps[i])) {
      // Merge steps[i] into the last optimized step
      MergeSteps(optimized.back(), steps[i]);
    } else {
      // Can't merge, add as new step
      optimized.push_back(steps[i]);
    }
  }

  steps = std::move(optimized);
}

}  // namespace

void StepMergingOptimizer::Run(ir::InstructionSet& instruction_set) const {
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
