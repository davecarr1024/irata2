#include "irata2/microcode/debug/decoder.h"

#include <algorithm>
#include <sstream>

namespace irata2::microcode::debug {

MicrocodeDecoder::MicrocodeDecoder(const output::MicrocodeProgram& program)
    : program_(program) {}

std::vector<std::string> MicrocodeDecoder::DecodeControlWord(
    __uint128_t control_word) const {
  std::vector<std::string> controls;
  for (size_t i = 0; i < program_.control_paths.size(); ++i) {
    if ((control_word >> i) & 1U) {
      controls.push_back(program_.control_paths[i]);
    }
  }
  return controls;
}

std::string MicrocodeDecoder::DecodeStatusBits(uint8_t status) const {
  if (status == 0) {
    return "default";
  }

  std::vector<std::string> flags;
  for (const auto& bit_def : program_.status_bits) {
    if ((status >> bit_def.bit) & 1U) {
      flags.push_back(bit_def.name);
    }
  }

  std::ostringstream result;
  for (size_t i = 0; i < flags.size(); ++i) {
    if (i > 0) result << ",";
    result << flags[i];
  }
  return result.str();
}

std::string MicrocodeDecoder::DumpProgram() const {
  std::ostringstream output;

  // Collect all unique opcode/step/status tuples from the table
  std::vector<output::MicrocodeKey> keys;
  for (const auto& [encoded_key, control_word] : program_.table) {
    output::MicrocodeKey key;
    key.opcode = (encoded_key >> 16) & 0xFF;
    key.step = (encoded_key >> 8) & 0xFF;
    key.status = encoded_key & 0xFF;
    keys.push_back(key);
  }

  // Sort by opcode, then status, then step
  std::sort(keys.begin(), keys.end(),
            [](const output::MicrocodeKey& a, const output::MicrocodeKey& b) {
              if (a.opcode != b.opcode) return a.opcode < b.opcode;
              if (a.status != b.status) return a.status < b.status;
              return a.step < b.step;
            });

  // Dump each entry
  uint8_t current_opcode = 255;
  uint8_t current_status = 255;
  for (const auto& key : keys) {
    // Print opcode header
    if (key.opcode != current_opcode) {
      if (current_opcode != 255) output << "\n";
      output << "opcode " << static_cast<int>(key.opcode) << ":\n";
      current_opcode = key.opcode;
      current_status = 255;
    }

    // Print status variant header
    if (key.status != current_status) {
      output << "  status " << DecodeStatusBits(key.status) << ":\n";
      current_status = key.status;
    }

    // Print step
    const uint32_t encoded_key = output::EncodeKey(key);
    const __uint128_t control_word = program_.table.at(encoded_key);
    const auto controls = DecodeControlWord(control_word);

    output << "    step " << static_cast<int>(key.step) << ": [";
    for (size_t i = 0; i < controls.size(); ++i) {
      if (i > 0) output << ", ";
      output << controls[i];
    }
    output << "]\n";
  }

  return output.str();
}

std::string MicrocodeDecoder::DumpInstruction(uint8_t opcode) const {
  std::ostringstream output;

  // Collect all entries for this opcode
  std::vector<output::MicrocodeKey> keys;
  for (const auto& [encoded_key, control_word] : program_.table) {
    output::MicrocodeKey key;
    key.opcode = (encoded_key >> 16) & 0xFF;
    key.step = (encoded_key >> 8) & 0xFF;
    key.status = encoded_key & 0xFF;
    if (key.opcode == opcode) {
      keys.push_back(key);
    }
  }

  if (keys.empty()) {
    output << "opcode " << static_cast<int>(opcode) << ": (no microcode)\n";
    return output.str();
  }

  // Sort by status, then step
  std::sort(keys.begin(), keys.end(),
            [](const output::MicrocodeKey& a, const output::MicrocodeKey& b) {
              if (a.status != b.status) return a.status < b.status;
              return a.step < b.step;
            });

  output << "opcode " << static_cast<int>(opcode) << ":\n";
  uint8_t current_status = 255;
  for (const auto& key : keys) {
    // Print status variant header
    if (key.status != current_status) {
      output << "  status " << DecodeStatusBits(key.status) << ":\n";
      current_status = key.status;
    }

    // Print step
    const uint32_t encoded_key = output::EncodeKey(key);
    const __uint128_t control_word = program_.table.at(encoded_key);
    const auto controls = DecodeControlWord(control_word);

    output << "    step " << static_cast<int>(key.step) << ": [";
    for (size_t i = 0; i < controls.size(); ++i) {
      if (i > 0) output << ", ";
      output << controls[i];
    }
    output << "]\n";
  }

  return output.str();
}

std::string MicrocodeDecoder::DumpProgramYaml() const {
  std::ostringstream output;

  // Collect all unique opcode/step/status tuples from the table
  std::vector<output::MicrocodeKey> keys;
  for (const auto& [encoded_key, control_word] : program_.table) {
    output::MicrocodeKey key;
    key.opcode = (encoded_key >> 16) & 0xFF;
    key.step = (encoded_key >> 8) & 0xFF;
    key.status = encoded_key & 0xFF;
    keys.push_back(key);
  }

  // Sort by opcode, then status, then step
  std::sort(keys.begin(), keys.end(),
            [](const output::MicrocodeKey& a, const output::MicrocodeKey& b) {
              if (a.opcode != b.opcode) return a.opcode < b.opcode;
              if (a.status != b.status) return a.status < b.status;
              return a.step < b.step;
            });

  if (keys.empty()) {
    return "opcodes: {}\n";
  }

  output << "opcodes:\n";
  uint8_t current_opcode = 255;
  uint8_t current_status = 255;

  for (const auto& key : keys) {
    // Print opcode header
    if (key.opcode != current_opcode) {
      output << "  " << static_cast<int>(key.opcode) << ":\n";
      current_opcode = key.opcode;
      current_status = 255;
    }

    // Print status variant header
    if (key.status != current_status) {
      output << "    status_" << DecodeStatusBits(key.status) << ":\n";
      output << "      steps:\n";
      current_status = key.status;
    }

    // Print step
    const uint32_t encoded_key = output::EncodeKey(key);
    const __uint128_t control_word = program_.table.at(encoded_key);
    const auto controls = DecodeControlWord(control_word);

    output << "        - stage: " << static_cast<int>(key.step) << "\n";
    output << "          controls:\n";
    for (const auto& control : controls) {
      output << "            - " << control << "\n";
    }
  }

  return output.str();
}

std::string MicrocodeDecoder::DumpInstructionYaml(uint8_t opcode) const {
  std::ostringstream output;

  // Collect all entries for this opcode
  std::vector<output::MicrocodeKey> keys;
  for (const auto& [encoded_key, control_word] : program_.table) {
    output::MicrocodeKey key;
    key.opcode = (encoded_key >> 16) & 0xFF;
    key.step = (encoded_key >> 8) & 0xFF;
    key.status = encoded_key & 0xFF;
    if (key.opcode == opcode) {
      keys.push_back(key);
    }
  }

  if (keys.empty()) {
    output << "opcode_" << static_cast<int>(opcode) << ": null\n";
    return output.str();
  }

  // Sort by status, then step
  std::sort(keys.begin(), keys.end(),
            [](const output::MicrocodeKey& a, const output::MicrocodeKey& b) {
              if (a.status != b.status) return a.status < b.status;
              return a.step < b.step;
            });

  output << "opcode_" << static_cast<int>(opcode) << ":\n";
  uint8_t current_status = 255;

  for (const auto& key : keys) {
    // Print status variant header
    if (key.status != current_status) {
      output << "  status_" << DecodeStatusBits(key.status) << ":\n";
      output << "    steps:\n";
      current_status = key.status;
    }

    // Print step
    const uint32_t encoded_key = output::EncodeKey(key);
    const __uint128_t control_word = program_.table.at(encoded_key);
    const auto controls = DecodeControlWord(control_word);

    output << "      - stage: " << static_cast<int>(key.step) << "\n";
    output << "        controls:\n";
    for (const auto& control : controls) {
      output << "          - " << control << "\n";
    }
  }

  return output.str();
}

}  // namespace irata2::microcode::debug
