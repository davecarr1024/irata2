#ifndef IRATA2_MICROCODE_IR_IRATA_INSTRUCTION_SET_H
#define IRATA2_MICROCODE_IR_IRATA_INSTRUCTION_SET_H

#include "irata2/hdl/cpu.h"
#include "irata2/microcode/ir/instruction_set.h"

namespace irata2::microcode::ir {

InstructionSet BuildIrataInstructionSet(const hdl::Cpu& cpu);

}  // namespace irata2::microcode::ir

#endif  // IRATA2_MICROCODE_IR_IRATA_INSTRUCTION_SET_H
