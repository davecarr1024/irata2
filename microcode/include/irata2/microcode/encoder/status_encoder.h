#ifndef IRATA2_MICROCODE_ENCODER_STATUS_ENCODER_H
#define IRATA2_MICROCODE_ENCODER_STATUS_ENCODER_H

#include "irata2/microcode/error.h"
#include "irata2/microcode/output/program.h"

#include <cstdint>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

namespace irata2::microcode::encoder {

class StatusEncoder {
 public:
  explicit StatusEncoder(std::vector<output::StatusBitDefinition> bits);

  std::vector<uint8_t> ExpandPartial(
      const std::map<std::string, bool>& partial) const;
  std::map<std::string, bool> Decode(uint8_t status) const;

  const std::vector<output::StatusBitDefinition>& bits() const { return bits_; }

 private:
  std::vector<output::StatusBitDefinition> bits_;
  std::unordered_map<std::string, uint8_t> bit_by_name_;
};

}  // namespace irata2::microcode::encoder

#endif  // IRATA2_MICROCODE_ENCODER_STATUS_ENCODER_H
