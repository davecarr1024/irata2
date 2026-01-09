#ifndef IRATA2_MICROCODE_OUTPUT_PROGRAM_H
#define IRATA2_MICROCODE_OUTPUT_PROGRAM_H

#include "irata2/isa/isa.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace irata2::microcode::output {

struct MicrocodeKey {
  uint8_t opcode = 0;
  uint8_t step = 0;
  uint8_t status = 0;
};

inline uint32_t EncodeKey(MicrocodeKey key) {
  return (static_cast<uint32_t>(key.opcode) << 16) |
         (static_cast<uint32_t>(key.step) << 8) |
         static_cast<uint32_t>(key.status);
}

using MicrocodeTable = std::unordered_map<uint32_t, uint64_t>;

struct StatusBitDefinition {
  std::string name;
  uint8_t bit = 0;
};

struct MicrocodeProgram {
  MicrocodeTable table;
  std::vector<std::string> control_paths;
  std::vector<StatusBitDefinition> status_bits;
};

}  // namespace irata2::microcode::output

#endif  // IRATA2_MICROCODE_OUTPUT_PROGRAM_H
