#include "irata2/microcode/encoder/control_encoder.h"

#include "irata2/hdl/traits.h"

#include <sstream>

namespace irata2::microcode::encoder {

ControlEncoder::ControlEncoder(const hdl::Cpu& cpu) {
  // Build the control index via visitor traversal order.
  // Controls are indexed by their ControlInfo pointer, not by path string.
  size_t index = 0;
  cpu.visit([&](const auto& component) {
    using T = std::decay_t<decltype(component)>;
    if constexpr (hdl::is_control_v<T>) {
      const auto& info = component.control_info();
      control_paths_.emplace_back(info.path);
      control_index_.emplace(&info, index++);
    }
  });

  if (control_paths_.size() > 64) {
    std::ostringstream message;
    message << "too many controls for 64-bit control word: "
            << control_paths_.size();
    throw microcode::MicrocodeError(message.str());
  }
}

uint64_t ControlEncoder::Encode(
    const std::vector<const hdl::ControlInfo*>& controls) const {
  uint64_t word = 0;
  for (const auto* info : controls) {
    const auto it = control_index_.find(info);
    if (it == control_index_.end()) {
      std::ostringstream message;
      message << "control not registered: " << info->path;
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
