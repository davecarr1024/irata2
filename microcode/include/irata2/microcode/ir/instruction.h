#ifndef IRATA2_MICROCODE_IR_INSTRUCTION_H
#define IRATA2_MICROCODE_IR_INSTRUCTION_H

#include "irata2/isa/isa.h"
#include "irata2/microcode/ir/step.h"

#include <map>
#include <string>
#include <vector>

namespace irata2::microcode::ir {

struct InstructionVariant {
  std::map<std::string, bool> status_conditions;
  std::vector<Step> steps;
};

struct Instruction {
  isa::Opcode opcode;
  std::vector<InstructionVariant> variants;
};

}  // namespace irata2::microcode::ir

#endif  // IRATA2_MICROCODE_IR_INSTRUCTION_H
