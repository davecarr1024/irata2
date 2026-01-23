#include "irata2/sim/disassembler.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <sstream>
#include <unordered_map>

#include <fmt/format.h>

#include "irata2/isa/isa.h"

namespace irata2::sim {

namespace {
std::string FormatHexByte(uint8_t value) {
  return fmt::format("${:02X}", value);
}

std::string FormatHexWord(uint16_t value) {
  return fmt::format("${:04X}", value);
}

std::unordered_map<uint16_t, std::string> BuildLabelMap(const DebugSymbols* symbols) {
  std::unordered_map<uint16_t, std::string> labels;
  if (!symbols) {
    return labels;
  }
  for (const auto& entry : symbols->symbols) {
    const uint16_t address = entry.second.value();
    if (labels.find(address) == labels.end()) {
      labels.emplace(address, entry.first);
    }
  }
  return labels;
}

std::string FormatOperand(isa::AddressingMode mode,
                          const std::vector<uint8_t>& bytes,
                          uint16_t address,
                          const std::unordered_map<uint16_t, std::string>& labels) {
  auto label_for = [&](uint16_t target) -> std::optional<std::string> {
    auto it = labels.find(target);
    if (it == labels.end()) {
      return std::nullopt;
    }
    return it->second;
  };

  switch (mode) {
    case isa::AddressingMode::IMP:
      return "";
    case isa::AddressingMode::IMM:
      return "#" + FormatHexByte(bytes.at(0));
    case isa::AddressingMode::ZP:
      return FormatHexByte(bytes.at(0));
    case isa::AddressingMode::ZPX:
      return FormatHexByte(bytes.at(0)) + ",X";
    case isa::AddressingMode::ZPY:
      return FormatHexByte(bytes.at(0)) + ",Y";
    case isa::AddressingMode::IZX:
      return "(" + FormatHexByte(bytes.at(0)) + ",X)";
    case isa::AddressingMode::IZY:
      return "(" + FormatHexByte(bytes.at(0)) + "),Y";
    case isa::AddressingMode::ABS:
    case isa::AddressingMode::ABX:
    case isa::AddressingMode::ABY:
    case isa::AddressingMode::IND: {
      const uint16_t target = static_cast<uint16_t>(bytes.at(0) |
                                                   (static_cast<uint16_t>(bytes.at(1)) << 8));
      std::string base = FormatHexWord(target);
      if (auto label = label_for(target)) {
        base = *label;
      }
      if (mode == isa::AddressingMode::ABX) {
        return base + ",X";
      }
      if (mode == isa::AddressingMode::ABY) {
        return base + ",Y";
      }
      if (mode == isa::AddressingMode::IND) {
        return "(" + base + ")";
      }
      return base;
    }
    case isa::AddressingMode::REL: {
      const int8_t offset = static_cast<int8_t>(bytes.at(0));
      const uint16_t target = static_cast<uint16_t>(address + 2 + offset);
      if (auto label = label_for(target)) {
        return *label;
      }
      return FormatHexWord(target);
    }
  }
  return "";
}
}  // namespace

std::string Disassemble(const std::vector<base::Byte>& rom,
                        const DebugSymbols* symbols,
                        const DisassembleOptions& options) {
  std::ostringstream out;
  const auto labels = BuildLabelMap(symbols);

  if (options.emit_org) {
    out << ".org " << FormatHexWord(options.origin.value()) << "\n";
  }

  size_t idx = 0;
  uint16_t address = options.origin.value();

  while (idx < rom.size()) {
    const uint8_t opcode = rom[idx].value();
    if (options.emit_labels) {
      auto it = labels.find(address);
      if (it != labels.end()) {
        out << it->second << ":\n";
      }
    }

    const auto info = isa::IsaInfo::GetInstruction(opcode);
    if (!info) {
      out << ".byte " << FormatHexByte(opcode) << "\n";
      ++idx;
      ++address;
      continue;
    }

    const auto mode_info = isa::IsaInfo::GetAddressingMode(info->addressing_mode);
    const uint8_t operand_size = mode_info ? mode_info->operand_bytes : 0;
    if (idx + operand_size >= rom.size()) {
      out << ".byte " << FormatHexByte(opcode) << "\n";
      ++idx;
      ++address;
      continue;
    }

    std::vector<uint8_t> operand_bytes;
    operand_bytes.reserve(operand_size);
    for (uint8_t i = 0; i < operand_size; ++i) {
      operand_bytes.push_back(rom[idx + 1 + i].value());
    }

    std::string line = std::string(info->mnemonic);
    const std::string operand = FormatOperand(info->addressing_mode,
                                              operand_bytes,
                                              address,
                                              labels);
    if (!operand.empty()) {
      line += " " + operand;
    }

    if (options.emit_addresses || options.emit_bytes) {
      line += " ;";
      if (options.emit_addresses) {
        line += " " + FormatHexWord(address);
      }
      if (options.emit_bytes) {
        line += " " + FormatHexByte(opcode);
        for (uint8_t value : operand_bytes) {
          line += " " + FormatHexByte(value);
        }
      }
    }

    out << line << "\n";

    idx += static_cast<size_t>(1 + operand_size);
    address = static_cast<uint16_t>(address + 1 + operand_size);
  }

  return out.str();
}

}  // namespace irata2::sim
