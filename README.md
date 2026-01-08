# IRATA2 - 8-bit CPU Simulator

A cycle-accurate 8-bit CPU simulator in C++ that feels like building a breadboard computer.

## Project Structure

```
irata2/
├── base/                      # Common base module with fundamental types
│   ├── include/irata2/base/
│   │   ├── types.h           # Byte and Word types
│   │   └── tick_phase.h      # Five-phase tick model enum
│   └── CMakeLists.txt        # Independent build configuration
├── hdl/                       # Hardware Definition Language (structural metadata)
│   ├── include/irata2/hdl/
│   │   ├── component.h       # HDL component base classes
│   │   └── cpu.h             # HDL CPU structure
│   ├── src/
│   │   └── cpu.cpp
│   └── CMakeLists.txt        # Independent build configuration
├── sim/                       # Runtime simulator
│   ├── include/irata2/sim/
│   │   ├── component.h       # Simulator component base classes
│   │   └── cpu.h             # Simulator CPU with runtime state
│   ├── src/
│   │   └── cpu.cpp
│   └── CMakeLists.txt        # Independent build configuration
├── asm/                       # ISA module (YAML definition + code generation)
│   ├── instructions.yaml     # ISA definition (single source of truth)
│   ├── generate_isa.py       # Python code generator
│   ├── example_usage.cpp     # Example program
│   ├── README.md             # Module documentation
│   └── CMakeLists.txt        # Independent build configuration
├── microcode/                 # Microcode DSL, IR, compiler, encoder
│   ├── include/irata2/microcode/
│   └── CMakeLists.txt        # Independent build configuration
├── assembler/                 # Python assembler (to be implemented)
├── test/                      # Test harnesses, matchers, .asm programs
└── CMakeLists.txt            # Root build configuration
```

## Module Organization

### Base Module (`irata2::base`)
Common types and enums used across all modules:
- `Byte` - 8-bit value type (0x00 - 0xFF)
- `Word` - 16-bit value type (0x0000 - 0xFFFF)
- `TickPhase` - Five-phase tick model enum

### HDL Module (`irata2::hdl`)
Immutable structural metadata representing the CPU architecture:
- `Component` - Base class for all HDL components
- `ComponentWithParent` - Base for non-root components
- `Cpu` - Root HDL component

### Simulator Module (`irata2::sim`)
Runtime execution with mutable state:
- `Component` - Base class with tick phases
- `ComponentWithParent` - Base for non-root components
- `Cpu` - Root simulator with five-phase tick orchestration

### ASM Module (`irata2::asm`)
ISA definition and code generation:
- Single YAML file defines entire instruction set
- Python generator creates type-safe C++ headers
- Automatic code generation during build
- See [asm/README.md](asm/README.md) for details

### Microcode Module (`irata2::microcode`)
Microcode DSL, IR, compiler, and encoder (to be implemented)

## Modular Architecture

Each module is **independent and self-contained**:
- Has its own `CMakeLists.txt` with build configuration
- Exports its own library (e.g., `irata2::base`, `irata2::hdl`)
- Can be built independently or as part of the whole project
- Declares dependencies explicitly via `target_link_libraries`

**Build order** (managed automatically by CMake):
1. `base` - No dependencies
2. `hdl` - Depends on `base`
3. `asm` - Depends on `base` (includes code generation)
4. `sim` - Depends on `base`, `hdl`
5. `microcode` - Depends on `base`, `hdl`, `asm`

## Design Philosophy

- **Hardware-ish**: Components compose like hardware modules
- **Strongly Typed**: Invalid states are unrepresentable
- **Namespaced**: Each module in its own namespace
- **Header-per-Class**: Each class has its own header file
- **Testable**: 100% test coverage goal

## Building

```bash
mkdir build
cd build
cmake ..
make
```

## Documentation

See [design.md](design.md) for detailed architecture and design decisions.
