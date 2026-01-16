#include "irata2/sim/controller/control_encoder.h"

#include "irata2/sim/cpu.h"
#include "irata2/sim/error.h"

#include <sstream>

namespace irata2::sim::controller {

ControlEncoder::ControlEncoder(std::string name, Component& parent)
    : ComponentWithParent(parent, std::move(name)), control_word_width_(0) {}

void ControlEncoder::Initialize(
    const microcode::output::MicrocodeProgram& program,
    Cpu& cpu) {
  if (program.control_paths.size() > 64) {
    std::ostringstream message;
    message << "too many controls for instruction memory: "
            << program.control_paths.size();
    throw SimError(message.str());
  }

  control_references_.clear();
  control_references_.reserve(program.control_paths.size());

  const auto& control_order = cpu.ControlOrder();
  if (control_order.size() != program.control_paths.size()) {
    throw SimError("control table size mismatch between HDL and microcode");
  }

  for (size_t i = 0; i < control_order.size(); ++i) {
    auto* control = control_order[i];
    const auto& expected = program.control_paths[i];
    if (control->path() != expected) {
      throw SimError("control path order mismatch: " + expected);
    }
    control_references_.push_back(control);
  }

  // Calculate control word width in bytes
  const size_t num_controls = control_references_.size();
  control_word_width_ = (num_controls + 7) / 8;  // Round up to next byte
}

std::vector<uint8_t> ControlEncoder::Encode(
    const std::vector<ControlBase*>& controls) const {
  std::vector<uint8_t> word(control_word_width_, 0);

  for (auto* control : controls) {
    // Find the control's index
    bool found = false;
    for (size_t i = 0; i < control_references_.size(); ++i) {
      if (control_references_[i] == control) {
        // Set the corresponding bit
        const size_t byte_index = i / 8;
        const size_t bit_index = i % 8;
        word[byte_index] |= static_cast<uint8_t>(1U << bit_index);
        found = true;
        break;
      }
    }
    if (!found) {
      throw SimError("control not found in encoder: " + control->path());
    }
  }

  return word;
}

std::vector<ControlBase*> ControlEncoder::Decode(
    const std::vector<uint8_t>& word) const {
  if (word.size() != control_word_width_) {
    std::ostringstream message;
    message << "control word size mismatch: expected " << control_word_width_
            << " bytes, got " << word.size();
    throw SimError(message.str());
  }

  std::vector<ControlBase*> controls;

  for (size_t i = 0; i < control_references_.size(); ++i) {
    const size_t byte_index = i / 8;
    const size_t bit_index = i % 8;
    if ((word[byte_index] >> bit_index) & 1U) {
      controls.push_back(control_references_[i]);
    }
  }

  return controls;
}

}  // namespace irata2::sim::controller
