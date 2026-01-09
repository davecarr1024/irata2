#include "irata2/microcode/encoder/status_encoder.h"

#include <sstream>

namespace irata2::microcode::encoder {

StatusEncoder::StatusEncoder(std::vector<output::StatusBitDefinition> bits)
    : bits_(std::move(bits)) {
  for (const auto& bit : bits_) {
    if (bit.bit >= 8) {
      std::ostringstream message;
      message << "status bit out of range: " << static_cast<int>(bit.bit);
      throw microcode::MicrocodeError(message.str());
    }
    bit_by_name_.emplace(bit.name, bit.bit);
  }
}

std::vector<uint8_t> StatusEncoder::ExpandPartial(
    const std::map<std::string, bool>& partial) const {
  if (bits_.empty()) {
    if (!partial.empty()) {
      throw microcode::MicrocodeError("status bits not configured");
    }
    return {0};
  }

  uint8_t base = 0;
  std::vector<uint8_t> unspecified_bits;
  for (const auto& bit : bits_) {
    const auto it = partial.find(bit.name);
    if (it == partial.end()) {
      unspecified_bits.push_back(bit.bit);
    } else if (it->second) {
      base |= static_cast<uint8_t>(1U << bit.bit);
    }
  }

  for (const auto& [name, value] : partial) {
    if (!bit_by_name_.contains(name)) {
      std::ostringstream message;
      message << "unknown status: " << name;
      throw microcode::MicrocodeError(message.str());
    }
    (void)value;
  }

  std::vector<uint8_t> expanded;
  const size_t permutations = size_t{1} << unspecified_bits.size();
  expanded.reserve(permutations);
  for (size_t mask = 0; mask < permutations; ++mask) {
    uint8_t value = base;
    for (size_t i = 0; i < unspecified_bits.size(); ++i) {
      if (mask & (size_t{1} << i)) {
        value |= static_cast<uint8_t>(1U << unspecified_bits[i]);
      }
    }
    expanded.push_back(value);
  }

  return expanded;
}

std::map<std::string, bool> StatusEncoder::Decode(uint8_t status) const {
  if (bits_.empty()) {
    if (status != 0) {
      throw microcode::MicrocodeError("status bits not configured");
    }
    return {};
  }

  std::map<std::string, bool> decoded;
  for (const auto& bit : bits_) {
    decoded[bit.name] = ((status >> bit.bit) & 1U) != 0;
  }
  return decoded;
}

}  // namespace irata2::microcode::encoder
