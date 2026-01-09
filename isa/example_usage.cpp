// Example usage of the generated ISA library
// This file demonstrates how to use the ISA definitions

#include <iostream>
#include "irata2/isa/isa.h"

using namespace irata2::isa;

void print_instruction(const InstructionInfo& inst) {
  std::cout << "Instruction: " << inst.mnemonic
            << " (" << ToString(inst.addressing_mode) << ")\n";
  std::cout << "  Opcode: 0x" << std::hex << static_cast<int>(inst.opcode) << std::dec << "\n";
  std::cout << "  Category: " << ToString(inst.category) << "\n";
  std::cout << "  Cycles: " << static_cast<int>(inst.cycles) << "\n";
  std::cout << "  Description: " << inst.description << "\n";

  if (!inst.flags_affected.empty()) {
    std::cout << "  Flags affected: ";
    for (const auto& flag : inst.flags_affected) {
      std::cout << ToString(flag) << " ";
    }
    std::cout << "\n";
  }
  std::cout << "\n";
}

int main() {
  std::cout << "=== IRATA2 ISA Example Usage ===\n\n";

  // Example 1: Look up instruction by opcode value
  std::cout << "1. Looking up instruction by opcode value (0x01):\n";
  auto inst1 = IsaInfo::GetInstruction(0x01);
  if (inst1) {
    print_instruction(*inst1);
  }

  // Example 2: Use opcode enum
  std::cout << "2. Looking up instruction by opcode enum (NOP_IMP):\n";
  auto inst2 = IsaInfo::GetInstruction(Opcode::NOP_IMP);
  if (inst2) {
    print_instruction(*inst2);
  }

  // Example 3: Addressing mode information (IMP)
  std::cout << "3. Addressing mode information (IMP):\n";
  auto mode = IsaInfo::GetAddressingMode(AddressingMode::IMP);
  if (mode) {
    std::cout << "  Name: " << mode->name << "\n";
    std::cout << "  Code: " << mode->code << "\n";
    std::cout << "  Operand bytes: " << static_cast<int>(mode->operand_bytes) << "\n";
    std::cout << "  Syntax: " << mode->syntax << "\n";
    std::cout << "  Description: " << mode->description << "\n";
  }
  std::cout << "\n";

  // Example 4: List all system instructions
  std::cout << "4. System instructions:\n";
  for (const auto& inst : IsaInfo::GetInstructions()) {
    if (inst.category == InstructionCategory::System) {
      std::cout << "  0x" << std::hex << static_cast<int>(inst.opcode) << std::dec
                << " - " << inst.mnemonic << ": " << inst.description << "\n";
    }
  }

  return 0;
}
