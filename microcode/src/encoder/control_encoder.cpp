#include "irata2/microcode/encoder/control_encoder.h"

#include <algorithm>
#include <sstream>

namespace irata2::microcode::encoder {

ControlEncoder::ControlEncoder(const hdl::Cpu& cpu)
    : control_paths_(cpu.AllControlPaths()) {
  std::sort(control_paths_.begin(), control_paths_.end());
  if (control_paths_.size() > 64) {
    std::ostringstream message;
    message << "too many controls for 64-bit control word: "
            << control_paths_.size();
    throw microcode::MicrocodeError(message.str());
  }

  for (size_t i = 0; i < control_paths_.size(); ++i) {
    control_index_by_path_.emplace(control_paths_[i], i);
  }
}

uint64_t ControlEncoder::Encode(
    const std::vector<const hdl::ControlBase*>& controls) const {
  uint64_t word = 0;
  for (const auto* control : controls) {
    const auto it = control_index_by_path_.find(control->path());
    if (it == control_index_by_path_.end()) {
      std::ostringstream message;
      message << "control not registered: " << control->path();
      throw microcode::MicrocodeError(message.str());
    }
    word |= (uint64_t{1} << it->second);
  }
  return word;
}

std::vector<std::string> ControlEncoder::Decode(uint64_t control_word) const {
  std::vector<std::string> controls;
  for (size_t i = 0; i < control_paths_.size(); ++i) {
    if ((control_word >> i) & 1U) {
      controls.push_back(control_paths_[i]);
    }
  }
  return controls;
}

}  // namespace irata2::microcode::encoder
