# ISA Module - IRATA2 Instruction Set Architecture

The ISA module provides the single source of truth for the IRATA2 instruction set architecture. It uses a YAML definition file to generate C++ code, ensuring consistency across the simulator, assembler, and any other tools.

Current MVP status: the instruction set is intentionally minimal (HLT/NOP/CRS) to support the vertical slice. Additional instructions and addressing modes will be added after the end-to-end test harness is in place.

## Structure

```
isa/
├── instructions.yaml      # ISA definition (single source of truth)
├── generate_isa.py        # Python code generator
├── example_usage.cpp      # Example program
├── test/                  # Unit tests
└── README.md              # This file
```

## instructions.yaml

The YAML file defines three main sections:

### 1. Addressing Modes
Defines how instructions access memory and operands.

```yaml
addressing_modes:
  - name: Implied
    code: IMP
    operands: 0
    description: "No operands"
```

### 2. Status Flags
CPU status flags that can be affected by instructions.

```yaml
status_flags:
  - name: Zero
    code: Z
    bit: 0
    description: "Set if result is zero"
```

### 3. Instructions
Complete instruction set with opcodes, addressing modes, and metadata.

```yaml
instructions:
  - mnemonic: HLT
    opcode: 0x01
    addressing_mode: IMP
    cycles: 1
    description: "Halt execution"
    category: System
    flags_affected: []
```

## Code Generation

The Python script `generate_isa.py` reads `instructions.yaml` and generates `isa.h` containing:

1. **Enums**:
   - `AddressingMode` - All addressing mode codes
   - `StatusFlag` - All status flags with bit positions
   - `InstructionCategory` - Instruction categories (Load, Store, Arithmetic, etc.)
   - `Opcode` - All instruction opcodes (e.g., `LDA_IMM`, `STA_ABS`)

2. **Structs**:
   - `AddressingModeInfo` - Details about each addressing mode
   - `InstructionInfo` - Complete information about each instruction

3. **IsaInfo Class**:
   - Static lookup methods for instructions and addressing modes
   - `GetInstructions()` - Get all instructions
   - `GetInstruction(opcode)` - Lookup by opcode value or enum
   - `GetAddressingModes()` - Get all addressing modes
   - `GetAddressingMode(mode)` - Lookup addressing mode info

## CMake Integration

The code generation is integrated into the CMake build system:

```cmake
# Automatically generates isa.h from instructions.yaml
add_custom_command(
  OUTPUT ${ISA_HEADER}
  COMMAND ${Python3_EXECUTABLE} ${ISA_GENERATOR} ${ISA_YAML} ${ISA_HEADER}
  DEPENDS ${ISA_YAML} ${ISA_GENERATOR}
)
```

To generate the ISA header manually:
```bash
cd build
make generate_isa
```

The generated header will be at: `build/isa/include/irata2/isa/isa.h`

## Usage

```cmake
target_link_libraries(your_target PRIVATE irata2::isa)
```

```cpp
#include "irata2/isa/isa.h"

using namespace irata2::isa;

// Lookup instruction by opcode
auto inst = IsaInfo::GetInstruction(0x01);  // HLT
if (inst) {
  std::cout << "Mnemonic: " << inst->mnemonic << "\n";
  std::cout << "Addressing: " << ToString(inst->addressing_mode) << "\n";
  std::cout << "Cycles: " << static_cast<int>(inst->cycles) << "\n";
  std::cout << "Category: " << ToString(inst->category) << "\n";
}

// Use opcode enum
auto inst2 = IsaInfo::GetInstruction(Opcode::NOP_IMP);

// Iterate all instructions
for (const auto& inst : IsaInfo::GetInstructions()) {
  if (inst.category == InstructionCategory::System) {
    std::cout << "System instruction: " << inst.mnemonic << "\n";
  }
}

// Get addressing mode info
auto mode = IsaInfo::GetAddressingMode(AddressingMode::IMP);
if (mode) {
  std::cout << "Operand bytes: " << static_cast<int>(mode->operand_bytes) << "\n";
  std::cout << "Syntax: " << mode->syntax << "\n";
}
```

## Adding New Instructions

1. Edit `isa/instructions.yaml`
2. Add your instruction with all required fields
3. Rebuild the project - the code will be regenerated automatically

Example:
```yaml
- mnemonic: HLT
  opcode: 0x01
  addressing_mode: IMP
  cycles: 1
  description: "Halt execution"
  category: System
  flags_affected: []
```

## Benefits

- **Single Source of Truth**: One YAML file defines the entire ISA
- **Type Safety**: Generated C++ enums prevent invalid opcodes/modes
- **Consistency**: Assembler and simulator use the same definitions
- **Maintainability**: Add instructions by editing YAML, not scattered C++ code
- **Documentation**: Descriptions and metadata embedded in the definition

## Future Extensions

The YAML-to-C++ generation can be extended to also generate:
- Python bindings for the assembler
- Documentation (markdown/HTML)
- Test case templates
- Disassembler tables
