#include "irata2/microcode/compiler/status_validator.h"

#include "irata2/microcode/error.h"

#include <set>
#include <sstream>

namespace irata2::microcode::compiler {

StatusValidator::StatusValidator(const encoder::StatusEncoder& status_encoder)
    : status_encoder_(status_encoder) {}

void StatusValidator::Run(ir::InstructionSet& instruction_set) const {
  const size_t num_status_bits = status_encoder_.bits().size();
  const size_t total_statuses = size_t{1} << num_status_bits;

  for (const auto& instruction : instruction_set.instructions) {
    // Single unconditional variant is valid
    if (instruction.variants.size() == 1 &&
        instruction.variants[0].status_conditions.empty()) {
      continue;
    }

    // No variants is valid (though unusual)
    if (instruction.variants.empty()) {
      continue;
    }

    // Collect all expanded statuses from all variants
    std::set<uint8_t> covered_statuses;

    for (const auto& variant : instruction.variants) {
      // Enforce single-status rule: each variant should specify at most one bit
      if (variant.status_conditions.size() > 1) {
        std::ostringstream message;
        message << "variant specifies multiple status bits in opcode "
                << static_cast<int>(instruction.opcode) << ": ";
        bool first = true;
        for (const auto& [name, value] : variant.status_conditions) {
          if (!first) message << ", ";
          message << name << "=" << (value ? "true" : "false");
          first = false;
        }
        throw MicrocodeError(message.str());
      }

      // Expand this variant's status specification
      const auto expanded = status_encoder_.ExpandPartial(variant.status_conditions);

      // Check for overlaps
      for (const auto status : expanded) {
        if (!covered_statuses.insert(status).second) {
          std::ostringstream message;
          message << "overlapping status coverage in opcode "
                  << static_cast<int>(instruction.opcode)
                  << ": status " << static_cast<int>(status)
                  << " covered by multiple variants";
          throw MicrocodeError(message.str());
        }
      }
    }

    // Check for complete coverage (no gaps)
    if (covered_statuses.size() != total_statuses) {
      std::ostringstream message;
      message << "incomplete status coverage in opcode "
              << static_cast<int>(instruction.opcode)
              << ": covered " << covered_statuses.size()
              << " of " << total_statuses << " possible statuses";

      // Find which statuses are missing
      std::vector<uint8_t> missing;
      for (size_t i = 0; i < total_statuses; ++i) {
        if (!covered_statuses.contains(static_cast<uint8_t>(i))) {
          missing.push_back(static_cast<uint8_t>(i));
        }
      }

      if (!missing.empty()) {
        message << " (missing:";
        for (size_t i = 0; i < std::min(missing.size(), size_t{5}); ++i) {
          if (i > 0) message << ",";
          message << " " << static_cast<int>(missing[i]);
        }
        if (missing.size() > 5) {
          message << " and " << (missing.size() - 5) << " more";
        }
        message << ")";
      }

      throw MicrocodeError(message.str());
    }
  }
}

}  // namespace irata2::microcode::compiler
