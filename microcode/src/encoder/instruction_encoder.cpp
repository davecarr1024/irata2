#include "irata2/microcode/encoder/instruction_encoder.h"

namespace irata2::microcode::encoder {

output::MicrocodeKey InstructionEncoder::MakeKey(isa::Opcode opcode,
                                                 uint8_t step,
                                                 uint8_t status) {
  output::MicrocodeKey key;
  key.opcode = static_cast<uint8_t>(opcode);
  key.step = step;
  key.status = status;
  return key;
}

output::MicrocodeKey InstructionEncoder::DecodeKey(uint32_t encoded) {
  output::MicrocodeKey key;
  key.opcode = static_cast<uint8_t>((encoded >> 16) & 0xFF);
  key.step = static_cast<uint8_t>((encoded >> 8) & 0xFF);
  key.status = static_cast<uint8_t>(encoded & 0xFF);
  return key;
}

}  // namespace irata2::microcode::encoder
