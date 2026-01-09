#ifndef IRATA2_MICROCODE_ENCODER_INSTRUCTION_ENCODER_H
#define IRATA2_MICROCODE_ENCODER_INSTRUCTION_ENCODER_H

#include "irata2/isa/isa.h"
#include "irata2/microcode/output/program.h"

#include <cstdint>

namespace irata2::microcode::encoder {

class InstructionEncoder {
 public:
  static output::MicrocodeKey MakeKey(isa::Opcode opcode,
                                     uint8_t step,
                                     uint8_t status);
  static output::MicrocodeKey DecodeKey(uint32_t encoded);
};

}  // namespace irata2::microcode::encoder

#endif  // IRATA2_MICROCODE_ENCODER_INSTRUCTION_ENCODER_H
