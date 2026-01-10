#include "irata2/microcode/compiler/sequence_transformer.h"

#include <algorithm>

namespace irata2::microcode::compiler {

SequenceTransformer::SequenceTransformer(const hdl::ControlInfo& increment_control,
                                         const hdl::ControlInfo& reset_control)
    : increment_control_(increment_control), reset_control_(reset_control) {}

void SequenceTransformer::Run(ir::InstructionSet& instruction_set) const {
  for (auto& instruction : instruction_set.instructions) {
    for (auto& variant : instruction.variants) {
      if (variant.steps.empty()) {
        continue;
      }

      for (size_t i = 0; i < variant.steps.size(); ++i) {
        const bool is_last = (i + 1 == variant.steps.size());
        auto& controls = variant.steps[i].controls;
        const hdl::ControlInfo& control_to_add =
            is_last ? reset_control_ : increment_control_;

        const bool already_present = std::any_of(
            controls.begin(), controls.end(),
            [&](const hdl::ControlInfo* info) { return info == &control_to_add; });

        if (!already_present) {
          controls.push_back(&control_to_add);
        }
      }
    }
  }
}

}  // namespace irata2::microcode::compiler
