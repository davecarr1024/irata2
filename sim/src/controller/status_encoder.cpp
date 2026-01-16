#include "irata2/sim/controller/status_encoder.h"

#include "irata2/sim/cpu.h"
#include "irata2/sim/error.h"
#include "irata2/sim/status_register.h"

#include <sstream>
#include <unordered_map>

namespace irata2::sim::controller {

StatusEncoder::StatusEncoder(std::string name, Component& parent)
    : ComponentWithParent(parent, std::move(name)) {}

void StatusEncoder::Initialize(
    const microcode::output::MicrocodeProgram& program,
    Cpu& cpu) {
  if (program.status_bits.size() > 8) {
    std::ostringstream message;
    message << "too many status bits: " << program.status_bits.size();
    throw SimError(message.str());
  }

  status_references_.clear();
  status_references_.reserve(program.status_bits.size());

  // Create a mapping from status bit names to Status references
  std::unordered_map<std::string, Status*> status_map;
  auto& status_reg = cpu.status();
  status_map["negative"] = &status_reg.negative();
  status_map["overflow"] = &status_reg.overflow();
  status_map["unused"] = &status_reg.unused();
  status_map["break"] = &status_reg.brk();
  status_map["decimal"] = &status_reg.decimal();
  status_map["interrupt_disable"] = &status_reg.interrupt_disable();
  status_map["zero"] = &status_reg.zero();
  status_map["carry"] = &status_reg.carry();

  for (const auto& bit_def : program.status_bits) {
    auto it = status_map.find(bit_def.name);
    if (it == status_map.end()) {
      throw SimError("unknown status bit: " + bit_def.name);
    }
    auto* status = it->second;

    // Verify bit index matches
    if (status->bit_index() != bit_def.bit) {
      std::ostringstream message;
      message << "status bit index mismatch for " << bit_def.name
              << ": expected " << static_cast<int>(bit_def.bit)
              << ", got " << static_cast<int>(status->bit_index());
      throw SimError(message.str());
    }

    status_references_.push_back(status);
  }
}

uint8_t StatusEncoder::Encode() const {
  uint8_t encoded = 0;
  for (size_t i = 0; i < status_references_.size(); ++i) {
    if (status_references_[i]->value()) {
      encoded |= static_cast<uint8_t>(1U << i);
    }
  }
  return encoded;
}

std::vector<bool> StatusEncoder::Decode(uint8_t encoded) const {
  std::vector<bool> values;
  values.reserve(status_references_.size());

  for (size_t i = 0; i < status_references_.size(); ++i) {
    values.push_back((encoded >> i) & 1U);
  }

  return values;
}

std::vector<uint8_t> StatusEncoder::Permute(uint8_t status_mask,
                                             uint8_t status_value) const {
  std::vector<uint8_t> results;

  const size_t num_status_bits = status_references_.size();
  if (num_status_bits == 0) {
    results.push_back(0);
    return results;
  }

  // Calculate the maximum encoding value based on number of status bits
  const size_t max_encoding = 1U << num_status_bits;

  // Generate all possible status encodings
  for (size_t encoding = 0; encoding < max_encoding; ++encoding) {
    const uint8_t enc = static_cast<uint8_t>(encoding);

    // Check if this encoding matches the specified bits
    if ((enc & status_mask) == (status_value & status_mask)) {
      results.push_back(enc);
    }
  }

  return results;
}

}  // namespace irata2::sim::controller
