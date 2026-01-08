// Example usage of the generated ISA library
// This file demonstrates how to use the ISA definitions

#include <iostream>
#include "irata2/asm/isa.h"

using namespace irata2::asm;

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
  std::cout << "1. Looking up instruction by opcode value (0xA0):\n";
  auto inst1 = IsaInfo::GetInstruction(0xA0);
  if (inst1) {
    print_instruction(*inst1);
  }

  // Example 2: Use opcode enum
  std::cout << "2. Looking up instruction by opcode enum (LDA_IMM):\n";
  auto inst2 = IsaInfo::GetInstruction(Opcode::LDA_IMM);
  if (inst2) {
    print_instruction(*inst2);
  }

  // Example 3: List all Load instructions
  std::cout << "3. All Load instructions:\n";
  for (const auto& inst : IsaInfo::GetInstructions()) {
    if (inst.category == InstructionCategory::Load) {
      std::cout << "  " << inst.mnemonic << " "
                << ToString(inst.addressing_mode) << "\n";
    }
  }
  std::cout << "\n";

  // Example 4: List all Branch instructions
  std::cout << "4. All Branch instructions:\n";
  for (const auto& inst : IsaInfo::GetInstructions()) {
    if (inst.category == InstructionCategory::Branch) {
      std::cout << "  " << inst.mnemonic << " - " << inst.description << "\n";
    }
  }
  std::cout << "\n";

  // Example 5: Get addressing mode information
  std::cout << "5. Addressing mode information (IMM):\n";
  auto mode = IsaInfo::GetAddressingMode(AddressingMode::IMM);
  if (mode) {
    std::cout << "  Name: " << mode->name << "\n";
    std::cout << "  Code: " << mode->code << "\n";
    std::cout << "  Operand bytes: " << static_cast<int>(mode->operand_bytes) << "\n";
    std::cout << "  Syntax: " << mode->syntax << "\n";
    std::cout << "  Description: " << mode->description << "\n";
  }
  std::cout << "\n";

  // Example 6: Count instructions by category
  std::cout << "6. Instruction count by category:\n";
  std::map<InstructionCategory, int> category_counts;
  for (const auto& inst : IsaInfo::GetInstructions()) {
    category_counts[inst.category]++;
  }
  for (const auto& [category, count] : category_counts) {
    std::cout << "  " << ToString(category) << ": " << count << " instructions\n";
  }
  std::cout << "\n";

  // Example 7: List all system instructions
  std::cout << "7. System instructions:\n";
  for (const auto& inst : IsaInfo::GetInstructions()) {
    if (inst.category == InstructionCategory::System) {
      std::cout << "  0x" << std::hex << static_cast<int>(inst.opcode) << std::dec
                << " - " << inst.mnemonic << ": " << inst.description << "\n";
    }
  }

  return 0;
}
