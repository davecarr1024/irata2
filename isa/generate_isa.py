#!/usr/bin/env python3
"""
Code generator for IRATA2 ISA C++ library.
Reads instructions.yaml and generates C++ header files.
"""

import yaml
import sys
from pathlib import Path
from typing import Dict, List, Any


def generate_header(data: Dict[str, Any]) -> str:
    """Generate the C++ header file content."""

    lines = [
        "// GENERATED CODE - DO NOT EDIT",
        "// Generated from isa/instructions.yaml",
        "",
        "#ifndef IRATA2_ISA_ISA_H",
        "#define IRATA2_ISA_ISA_H",
        "",
        "#include <cstdint>",
        "#include <string>",
        "#include <string_view>",
        "#include <map>",
        "#include <vector>",
        "#include <optional>",
        "",
        "namespace irata2::isa {",
        "",
    ]

    # Generate AddressingMode enum
    lines.extend(generate_addressing_mode_enum(data['addressing_modes']))
    lines.append("")

    # Generate StatusFlag enum
    lines.extend(generate_status_flag_enum(data['status_flags']))
    lines.append("")

    # Generate InstructionCategory enum
    lines.extend(generate_category_enum(data['instructions']))
    lines.append("")

    # Generate Opcode enum
    lines.extend(generate_opcode_enum(data['instructions']))
    lines.append("")

    # Generate AddressingModeInfo struct
    lines.extend(generate_addressing_mode_info(data['addressing_modes']))
    lines.append("")

    # Generate InstructionInfo struct
    lines.extend(generate_instruction_info())
    lines.append("")

    # Generate IsaInfo class with static lookup methods
    lines.extend(generate_isa_info_class(data))
    lines.append("")

    lines.extend([
        "}  // namespace irata2::isa",
        "",
        "#endif  // IRATA2_ISA_ISA_H",
        ""
    ])

    return "\n".join(lines)


def generate_addressing_mode_enum(addressing_modes: List[Dict[str, Any]]) -> List[str]:
    """Generate AddressingMode enum."""
    lines = [
        "// Addressing modes",
        "enum class AddressingMode : uint8_t {",
    ]

    for mode in addressing_modes:
        lines.append(f"  {mode['code']},  // {mode['name']}: {mode['description']}")

    lines.append("};")
    lines.append("")
    lines.append("// Convert AddressingMode to string")
    lines.append("inline std::string_view ToString(AddressingMode mode) {")
    lines.append("  switch (mode) {")

    for mode in addressing_modes:
        lines.append(f"    case AddressingMode::{mode['code']}: return \"{mode['name']}\";")

    lines.append("  }")
    lines.append("  return \"Unknown\";")
    lines.append("}")

    return lines


def generate_status_flag_enum(status_flags: List[Dict[str, Any]]) -> List[str]:
    """Generate StatusFlag enum."""
    lines = [
        "// Status flags",
        "enum class StatusFlag : uint8_t {",
    ]

    for flag in status_flags:
        lines.append(f"  {flag['code']} = {flag['bit']},  // {flag['name']}: {flag['description']}")

    lines.append("};")
    lines.append("")
    lines.append("// Convert StatusFlag to string")
    lines.append("inline std::string_view ToString(StatusFlag flag) {")
    lines.append("  switch (flag) {")

    for flag in status_flags:
        lines.append(f"    case StatusFlag::{flag['code']}: return \"{flag['name']}\";")

    lines.append("  }")
    lines.append("  return \"Unknown\";")
    lines.append("}")

    return lines


def generate_category_enum(instructions: List[Dict[str, Any]]) -> List[str]:
    """Generate InstructionCategory enum from unique categories."""
    categories = sorted(set(inst['category'] for inst in instructions))

    lines = [
        "// Instruction categories",
        "enum class InstructionCategory {",
    ]

    for category in categories:
        lines.append(f"  {category},")

    lines.append("};")
    lines.append("")
    lines.append("// Convert InstructionCategory to string")
    lines.append("inline std::string_view ToString(InstructionCategory category) {")
    lines.append("  switch (category) {")

    for category in categories:
        lines.append(f"    case InstructionCategory::{category}: return \"{category}\";")

    lines.append("  }")
    lines.append("  return \"Unknown\";")
    lines.append("}")

    return lines


def generate_opcode_enum(instructions: List[Dict[str, Any]]) -> List[str]:
    """Generate Opcode enum."""
    lines = [
        "// Instruction opcodes",
        "enum class Opcode : uint8_t {",
    ]

    # Group by mnemonic for better readability
    by_mnemonic: Dict[str, List[Dict[str, Any]]] = {}
    for inst in instructions:
        if inst['mnemonic'] not in by_mnemonic:
            by_mnemonic[inst['mnemonic']] = []
        by_mnemonic[inst['mnemonic']].append(inst)

    for mnemonic in sorted(by_mnemonic.keys()):
        insts = by_mnemonic[mnemonic]
        lines.append(f"  // {mnemonic}")
        for inst in insts:
            name = f"{inst['mnemonic']}_{inst['addressing_mode']}"
            lines.append(f"  {name} = {inst['opcode']:#04x},")
        lines.append("")

    lines.append("};")
    lines.append("")
    lines.append("// Convert Opcode to string")
    lines.append("inline std::string ToString(Opcode opcode) {")
    lines.append("  switch (opcode) {")

    for inst in instructions:
        name = f"{inst['mnemonic']}_{inst['addressing_mode']}"
        lines.append(f"    case Opcode::{name}: return \"{name}\";")

    lines.append("  }")
    lines.append("  return \"Unknown\";")
    lines.append("}")

    return lines


def generate_addressing_mode_info(addressing_modes: List[Dict[str, Any]]) -> List[str]:
    """Generate AddressingModeInfo struct."""
    lines = [
        "// Information about an addressing mode",
        "struct AddressingModeInfo {",
        "  AddressingMode mode;",
        "  std::string_view name;",
        "  std::string_view code;",
        "  uint8_t operand_bytes;",
        "  std::string_view description;",
        "  std::string_view syntax;",
        "};",
    ]

    return lines


def generate_instruction_info() -> List[str]:
    """Generate InstructionInfo struct."""
    lines = [
        "// Information about an instruction",
        "struct InstructionInfo {",
        "  Opcode opcode;",
        "  std::string_view mnemonic;",
        "  AddressingMode addressing_mode;",
        "  uint8_t cycles;",
        "  std::string_view description;",
        "  InstructionCategory category;",
        "  std::vector<StatusFlag> flags_affected;",
        "};",
    ]

    return lines


def generate_isa_info_class(data: Dict[str, Any]) -> List[str]:
    """Generate IsaInfo class with static lookup methods."""
    lines = [
        "// Static ISA information lookup",
        "class IsaInfo {",
        " public:",
        "  // Get all addressing modes",
        "  static const std::vector<AddressingModeInfo>& GetAddressingModes() {",
        "    static const std::vector<AddressingModeInfo> modes = {",
    ]

    # Generate addressing mode data
    for mode in data['addressing_modes']:
        syntax = mode.get('syntax', '')
        lines.append(f"      {{AddressingMode::{mode['code']}, \"{mode['name']}\", "
                    f"\"{mode['code']}\", {mode['operands']}, \"{mode['description']}\", "
                    f"\"{syntax}\"}},")

    lines.extend([
        "    };",
        "    return modes;",
        "  }",
        "",
        "  // Get all instructions",
        "  static const std::vector<InstructionInfo>& GetInstructions() {",
        "    static const std::vector<InstructionInfo> instructions = {",
    ])

    # Generate instruction data
    for inst in data['instructions']:
        opcode_name = f"{inst['mnemonic']}_{inst['addressing_mode']}"
        flags = ", ".join(f"StatusFlag::{f}" for f in inst.get('flags_affected', []))
        flags_vec = f"{{{flags}}}" if flags else "{}"

        lines.append(f"      {{Opcode::{opcode_name}, \"{inst['mnemonic']}\", "
                    f"AddressingMode::{inst['addressing_mode']}, {inst['cycles']}, "
                    f"\"{inst['description']}\", InstructionCategory::{inst['category']}, "
                    f"{flags_vec}}},")

    lines.extend([
        "    };",
        "    return instructions;",
        "  }",
        "",
        "  // Get instruction info by opcode value",
        "  static std::optional<InstructionInfo> GetInstruction(uint8_t opcode_value) {",
        "    static const std::map<uint8_t, InstructionInfo> opcode_map = [] {",
        "      std::map<uint8_t, InstructionInfo> map;",
        "      for (const auto& inst : GetInstructions()) {",
        "        map[static_cast<uint8_t>(inst.opcode)] = inst;",
        "      }",
        "      return map;",
        "    }();",
        "",
        "    auto it = opcode_map.find(opcode_value);",
        "    if (it != opcode_map.end()) {",
        "      return it->second;",
        "    }",
        "    return std::nullopt;",
        "  }",
        "",
        "  // Get instruction info by opcode enum",
        "  static std::optional<InstructionInfo> GetInstruction(Opcode opcode) {",
        "    return GetInstruction(static_cast<uint8_t>(opcode));",
        "  }",
        "",
        "  // Get addressing mode info by mode enum",
        "  static std::optional<AddressingModeInfo> GetAddressingMode(AddressingMode mode) {",
        "    for (const auto& info : GetAddressingModes()) {",
        "      if (info.mode == mode) {",
        "        return info;",
        "      }",
        "    }",
        "    return std::nullopt;",
        "  }",
        "};",
    ])

    return lines


def main():
    if len(sys.argv) != 3:
        print(f"Usage: {sys.argv[0]} <input_yaml> <output_header>", file=sys.stderr)
        sys.exit(1)

    input_file = Path(sys.argv[1])
    output_file = Path(sys.argv[2])

    # Read YAML
    with open(input_file, 'r') as f:
        data = yaml.safe_load(f)

    # Generate C++ header
    header_content = generate_header(data)

    # Write output
    output_file.parent.mkdir(parents=True, exist_ok=True)
    with open(output_file, 'w') as f:
        f.write(header_content)

    print(f"Generated {output_file} from {input_file}")


if __name__ == '__main__':
    main()
