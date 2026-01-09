#include "irata2/microcode/compiler/isa_coverage_validator.h"

#include "irata2/isa/isa.h"
#include "irata2/microcode/error.h"

#include <set>
#include <sstream>

namespace irata2::microcode::compiler {

void IsaCoverageValidator::Run(ir::InstructionSet& instruction_set) const {
  std::set<isa::Opcode> expected;
  for (const auto& info : isa::IsaInfo::GetInstructions()) {
    expected.insert(info.opcode);
  }

  std::set<isa::Opcode> seen;
  for (const auto& instruction : instruction_set.instructions) {
    if (!expected.contains(instruction.opcode)) {
      std::ostringstream message;
      message << "microcode defines unknown opcode "
              << static_cast<int>(instruction.opcode);
      throw MicrocodeError(message.str());
    }
    if (!seen.insert(instruction.opcode).second) {
      std::ostringstream message;
      message << "duplicate microcode for opcode "
              << static_cast<int>(instruction.opcode);
      throw MicrocodeError(message.str());
    }
  }

  if (seen.size() != expected.size()) {
    std::ostringstream message;
    message << "microcode does not implement all ISA instructions"
            << " (expected " << expected.size() << ", got " << seen.size() << ")";
    throw MicrocodeError(message.str());
  }
}

}  // namespace irata2::microcode::compiler
