#ifndef IRATA2_MICROCODE_IR_INSTRUCTION_SET_H
#define IRATA2_MICROCODE_IR_INSTRUCTION_SET_H

#include "irata2/microcode/ir/instruction.h"

#include <vector>

namespace irata2::microcode::ir {

struct InstructionSet {
  std::vector<Step> fetch_preamble;
  std::vector<Instruction> instructions;
};

}  // namespace irata2::microcode::ir

#endif  // IRATA2_MICROCODE_IR_INSTRUCTION_SET_H
