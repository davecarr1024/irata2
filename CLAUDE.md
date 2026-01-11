# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

```bash
# Standard build with tests
cmake -B build -DBUILD_TESTING=ON
cmake --build build --parallel

# Run all tests
ctest --test-dir build --output-on-failure -j 8

# Run a single test binary
./build/base/test/base_tests
./build/hdl/test/hdl_tests
./build/sim/test/sim_tests
./build/isa/test/isa_tests

# Run specific test by filter
./build/base/test/base_tests --gtest_filter="ByteTest.*"
./build/hdl/test/hdl_tests --gtest_filter="CpuTest.*"

# Build with coverage
cmake -B build -DENABLE_COVERAGE=ON -DBUILD_TESTING=ON
cmake --build build --parallel
ctest --test-dir build -j 8
./scripts/coverage.sh build
# Report at: build/coverage/html/index.html

# Release build
cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF
cmake --build build --parallel
```

Notes:
- Prefer parallel builds/tests by default.
- Run build + tests before committing when practical.

## Project Overview

IRATA2 is a cycle-accurate 8-bit CPU simulator in C++20. The goal is a "hardware-ish" design where components compose like breadboard modules. All intelligence lives in microcode, not component implementations.

## Module Architecture

Modules build in dependency order:

```
base → hdl → isa → sim → microcode
         └────────────────┘
```

| Module | Purpose |
|--------|---------|
| `base` | Core types: `Byte`, `Word`, `TickPhase` |
| `hdl` | Immutable CPU structure (schematic, no runtime state) |
| `isa` | YAML-defined instruction set, generates C++ via Python |
| `sim` | Runtime simulator with mutable state and tick orchestration |
| `microcode` | DSL, IR, compiler, encoder (placeholder) |

## Key Design Patterns

### CRTP Component Hierarchy (HDL)

All HDL components use Curiously Recurring Template Pattern for zero-cost static dispatch. Parents are type-erased for reusability.

```cpp
template<typename Derived>
class Component : public ComponentBase {
  Derived& self() { return static_cast<Derived&>(*this); }
  // visit() calls self().visit_impl()
};
```

Key hierarchy: `Component` → `ComponentWithParent` → `ComponentWithBus` → `Register`, `Bus`, `Control`

### Five-Phase Tick Model

Each CPU cycle has five phases executed in order:
1. **Control** - Controller reads microcode, asserts control signals
2. **Write** - Components write to buses
3. **Read** - Components read from buses
4. **Process** - Internal updates (ALU, status flags)
5. **Clear** - Reset auto-clear controls

Controls are phase-aware: `WriteControl` (Write phase), `ReadControl` (Read phase), `ProcessControl` (Process phase).

### ISA Code Generation

Instructions are defined in `isa/instructions.yaml`. The build runs `isa/generate_isa.py` to produce `build/isa/include/irata2/isa/isa.h` with enums and lookup tables. To regenerate:

```bash
cmake --build build --target generate_isa
```

## Include Patterns

```cpp
#include "irata2/base/types.h"      // Byte, Word
#include "irata2/base/tick_phase.h" // TickPhase
#include "irata2/hdl/cpu.h"         // HDL component tree
#include "irata2/sim/cpu.h"         // Runtime simulator
#include "irata2/isa/isa.h"         // Generated ISA definitions
```

Namespaces: `irata2::base`, `irata2::hdl`, `irata2::sim`, `irata2::isa`

## Test Conventions

- Tests use GoogleTest with `TEST()` macro
- Test files: `<module>/test/<component>_test.cpp`
- Test naming: `TEST(ComponentTest, BehaviorDescription)`
- Target 100% code coverage

## Adding New Components

HDL components follow this pattern:
1. Inherit from appropriate CRTP base (`ComponentWithParent`, `ComponentWithBus`, etc.)
2. Store child components as `const` members
3. Implement `visit_impl()` for traversal
4. Register in parent's `visit_impl()`

Adding instructions:
1. Edit `isa/instructions.yaml`
2. Rebuild - code generation runs automatically
