#include "irata2/microcode/compiler/fetch_transformer.h"

namespace irata2::microcode::compiler {

void FetchTransformer::Run(ir::InstructionSet& instruction_set) const {
  for (auto& instruction : instruction_set.instructions) {
    for (auto& variant : instruction.variants) {
      if (!instruction_set.fetch_preamble.empty()) {
        for (auto& step : variant.steps) {
          ++step.stage;
        }

        std::vector<ir::Step> prefixed = instruction_set.fetch_preamble;
        prefixed.insert(prefixed.end(), variant.steps.begin(), variant.steps.end());
        variant.steps = std::move(prefixed);
      }
    }
  }
}

}  // namespace irata2::microcode::compiler
