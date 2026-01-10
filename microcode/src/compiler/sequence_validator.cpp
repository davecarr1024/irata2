#include "irata2/microcode/compiler/sequence_validator.h"

#include "irata2/microcode/error.h"

#include <algorithm>
#include <sstream>

namespace irata2::microcode::compiler {

SequenceValidator::SequenceValidator(const hdl::ControlInfo& increment_control,
                                     const hdl::ControlInfo& reset_control)
    : increment_control_(increment_control), reset_control_(reset_control) {}

void SequenceValidator::Run(ir::InstructionSet& instruction_set) const {
  for (const auto& instruction : instruction_set.instructions) {
    for (const auto& variant : instruction.variants) {
      if (variant.steps.empty()) {
        continue;
      }

      for (size_t i = 0; i < variant.steps.size(); ++i) {
        const bool is_last = (i + 1 == variant.steps.size());
        const auto& controls = variant.steps[i].controls;
        const hdl::ControlInfo& expected =
            is_last ? reset_control_ : increment_control_;

        const bool found = std::any_of(
            controls.begin(), controls.end(),
            [&](const hdl::ControlInfo* control) {
              return control == &expected;
            });

        if (!found) {
          std::ostringstream message;
          message << "sequence control missing for opcode "
                  << static_cast<int>(instruction.opcode)
                  << " at step " << i;
          throw MicrocodeError(message.str());
        }
      }
    }
  }
}

}  // namespace irata2::microcode::compiler
