# IRATA2 - 8-bit CPU Simulator

A cycle-accurate 8-bit CPU simulator in C++ that feels like building a breadboard computer.

## Project Structure

```
irata2/
├── base/              # Common base module with fundamental types
│   ├── include/irata2/base/
│   │   ├── types.h           # Byte and Word types
│   │   └── tick_phase.h      # Five-phase tick model enum
│   └── src/
├── hdl/               # Hardware Definition Language (structural metadata)
│   ├── include/irata2/hdl/
│   │   ├── component.h       # HDL component base classes
│   │   └── cpu.h             # HDL CPU structure
│   └── src/
│       └── cpu.cpp
├── sim/               # Runtime simulator
│   ├── include/irata2/sim/
│   │   ├── component.h       # Simulator component base classes
│   │   └── cpu.h             # Simulator CPU with runtime state
│   └── src/
│       └── cpu.cpp
├── microcode/         # Microcode DSL, IR, compiler, encoder
│   ├── include/irata2/microcode/
│   └── src/
├── asm/               # ISA definitions (YAML)
├── assembler/         # Python assembler
├── test/              # Test harnesses, matchers, .asm programs
└── tools/             # Code generators
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

### Microcode Module (`irata2::microcode`)
Microcode DSL, IR, compiler, and encoder (to be implemented)

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
