#include "irata2/microcode/compiler/control_conflict_validator.h"

#include "irata2/microcode/error.h"

#include <map>
#include <set>
#include <sstream>
#include <string>

namespace irata2::microcode::compiler {

namespace {

// Extract component path (e.g., "a" from "a.read", "status.zero" from "status.zero.set")
std::string GetComponentPath(std::string_view control_path) {
  size_t last_dot = control_path.rfind('.');
  if (last_dot == std::string_view::npos) {
    return std::string(control_path);
  }
  return std::string(control_path.substr(0, last_dot));
}

// Get operation type from control path
std::string GetOperation(std::string_view control_path) {
  size_t last_dot = control_path.rfind('.');
  if (last_dot == std::string_view::npos) {
    return "";
  }
  return std::string(control_path.substr(last_dot + 1));
}

void ValidateStep(const ir::Step& step, int opcode, int step_index) {
  // Track read/write operations per component
  std::map<std::string, std::set<std::string>> component_operations;

  for (const auto* control : step.controls) {
    const std::string_view path = control->path;
    const std::string component = GetComponentPath(path);
    const std::string operation = GetOperation(path);

    // Track component operations
    component_operations[component].insert(operation);
  }

  // Check for read+write conflicts on the same component
  for (const auto& [component, operations] : component_operations) {
    if (operations.contains("read") && operations.contains("write")) {
      std::ostringstream message;
      message << "conflicting read and write on component '" << component
              << "' in opcode " << opcode << " step " << step_index;
      throw MicrocodeError(message.str());
    }

    // Check for set+clear conflicts
    if (operations.contains("set") && operations.contains("clear")) {
      std::ostringstream message;
      message << "conflicting set and clear on component '" << component
              << "' in opcode " << opcode << " step " << step_index;
      throw MicrocodeError(message.str());
    }

    // Check for increment+decrement conflicts
    if (operations.contains("increment") && operations.contains("decrement")) {
      std::ostringstream message;
      message << "conflicting increment and decrement on component '" << component
              << "' in opcode " << opcode << " step " << step_index;
      throw MicrocodeError(message.str());
    }
  }
  // Note: Multiple ALU opcode bits being set is valid because the ALU opcode
  // is binary encoded (not one-hot). Multiple bits form a single opcode value.
}

}  // namespace

void ControlConflictValidator::Run(ir::InstructionSet& instruction_set) const {
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
