#ifndef IRATA2_MICROCODE_ENCODER_CONTROL_ENCODER_H
#define IRATA2_MICROCODE_ENCODER_CONTROL_ENCODER_H

#include "irata2/hdl/control_info.h"
#include "irata2/hdl/cpu.h"
#include "irata2/microcode/error.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace irata2::microcode::encoder {

/// Encodes control signals to bit positions via parallel visitor traversal.
/// No string lookups are used for encoding - the ControlInfo pointer is
/// used directly as a key.
class ControlEncoder {
 public:
  explicit ControlEncoder(const hdl::Cpu& cpu);

  uint64_t Encode(const std::vector<const hdl::ControlInfo*>& controls) const;
  std::vector<std::string> Decode(uint64_t control_word) const;

  const std::vector<std::string>& control_paths() const { return control_paths_; }

 private:
  std::vector<std::string> control_paths_;
  std::unordered_map<const hdl::ControlInfo*, size_t> control_index_;
};

}  // namespace irata2::microcode::encoder

#endif  // IRATA2_MICROCODE_ENCODER_CONTROL_ENCODER_H
