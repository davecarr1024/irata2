#ifndef IRATA2_MICROCODE_ENCODER_CONTROL_ENCODER_H
#define IRATA2_MICROCODE_ENCODER_CONTROL_ENCODER_H

#include "irata2/hdl/control_base.h"
#include "irata2/hdl/cpu.h"
#include "irata2/microcode/error.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace irata2::microcode::encoder {

class ControlEncoder {
 public:
  explicit ControlEncoder(const hdl::Cpu& cpu);

  uint64_t Encode(const std::vector<const hdl::ControlBase*>& controls) const;
  std::vector<std::string> Decode(uint64_t control_word) const;

  const std::vector<std::string>& control_paths() const { return control_paths_; }

 private:
  std::vector<std::string> control_paths_;
  std::unordered_map<std::string, size_t> control_index_by_path_;
};

}  // namespace irata2::microcode::encoder

#endif  // IRATA2_MICROCODE_ENCODER_CONTROL_ENCODER_H
