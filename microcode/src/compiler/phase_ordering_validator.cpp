#include "irata2/microcode/compiler/phase_ordering_validator.h"

#include "irata2/base/tick_phase.h"
#include "irata2/microcode/error.h"

#include <sstream>

namespace irata2::microcode::compiler {

namespace {
bool IsReadControl(std::string_view path) {
  return path.find(".read") != std::string_view::npos;
}

bool IsWriteControl(std::string_view path) {
  return path.find(".write") != std::string_view::npos;
}

bool IsProcessControl(std::string_view path) {
  // Process controls include: increment, decrement, set, clear, latch
  return path.find(".increment") != std::string_view::npos ||
         path.find(".decrement") != std::string_view::npos ||
         path.find(".set") != std::string_view::npos ||
         path.find(".clear") != std::string_view::npos ||
         path.find(".latch") != std::string_view::npos;
}

void ValidateStep(const ir::Step& step, int opcode, int step_index) {
  for (const auto* control : step.controls) {
    const std::string_view path = control->path;
    const auto phase = control->phase;

    // Validate that read controls have Read phase
    if (IsReadControl(path) && phase != base::TickPhase::Read) {
      std::ostringstream message;
      message << "read control '" << path << "' in opcode "
              << opcode << " step " << step_index
              << " has phase " << static_cast<int>(phase)
              << ", expected Read phase (" << static_cast<int>(base::TickPhase::Read) << ")";
      throw MicrocodeError(message.str());
    }

    // Validate that write controls have Write phase
    if (IsWriteControl(path) && phase != base::TickPhase::Write) {
      std::ostringstream message;
      message << "write control '" << path << "' in opcode "
              << opcode << " step " << step_index
              << " has phase " << static_cast<int>(phase)
              << ", expected Write phase (" << static_cast<int>(base::TickPhase::Write) << ")";
      throw MicrocodeError(message.str());
    }

    // Validate that process controls have Process phase
    if (IsProcessControl(path) && phase != base::TickPhase::Process) {
      std::ostringstream message;
      message << "process control '" << path << "' in opcode "
              << opcode << " step " << step_index
              << " has phase " << static_cast<int>(phase)
              << ", expected Process phase (" << static_cast<int>(base::TickPhase::Process) << ")";
      throw MicrocodeError(message.str());
    }
  }
}

}  // namespace

void PhaseOrderingValidator::Run(ir::InstructionSet& instruction_set) const {
  // Validate fetch preamble
  for (size_t i = 0; i < instruction_set.fetch_preamble.size(); ++i) {
    ValidateStep(instruction_set.fetch_preamble[i], -1, static_cast<int>(i));
  }

  // Validate each instruction variant
  for (const auto& instruction : instruction_set.instructions) {
    for (const auto& variant : instruction.variants) {
      for (size_t i = 0; i < variant.steps.size(); ++i) {
        ValidateStep(variant.steps[i], static_cast<int>(instruction.opcode),
                     static_cast<int>(i));
      }
    }
  }
}

}  // namespace irata2::microcode::compiler
