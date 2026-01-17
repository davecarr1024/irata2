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
./build/microcode/test/microcode_tests
./build/assembler/test/assembler_tests

# Run specific test by filter
./build/base/test/base_tests --gtest_filter="ByteTest.*"
./build/hdl/test/hdl_tests --gtest_filter="CpuTest.*"

# Integration tests (with timeout protection against infinite loops)
./build/assembler/test/integration_tests
ctest --test-dir build -R ".*IntegrationTest.*" --timeout 120

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

### Makefile Shortcuts

```bash
make configure  # Configure cmake with tests
make build      # Build all targets
make test       # Run all tests
make coverage   # Build + test + generate coverage report
make clean      # Remove build directory
```

### Git Hooks

Enable pre-commit testing:
```bash
git config core.hooksPath .githooks
```

Notes:
- Prefer parallel builds/tests by default.
- Run build + tests before committing when practical.
- After pushing commits, watch CI and report failures.

## Project Overview

IRATA2 is a cycle-accurate 8-bit CPU simulator in C++20. The goal is a "hardware-ish" design where components compose like breadboard modules. All intelligence lives in microcode, not component implementations.

## Module Architecture

Modules build in dependency order (managed by CMake):

```
base → hdl → isa → sim → microcode → assembler
         └─────────────────────────────┘
```

| Module | Purpose |
|--------|---------|
| `base` | Core types: `Byte`, `Word`, `TickPhase` |
| `hdl` | Immutable CPU structure (schematic, no runtime state) |
| `isa` | YAML-defined instruction set, generates C++ via Python |
| `sim` | Runtime simulator with mutable state and tick orchestration |
| `microcode` | IR, compiler passes, validators, encoders for control sequences |
| `assembler` | Assembly language compiler, outputs cartridge ROM + debug JSON |

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

The sim module is organized into subdirectories with nested namespaces:
- `irata2::sim::alu` - ALU components
- `irata2::sim::controller` - Controller components
- `irata2::sim::memory` - Memory subsystem (RAM, ROM, regions)

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

## Documentation

- [docs/design.md](docs/design.md) - Full architecture and design philosophy
- [docs/plan.md](docs/plan.md) - Project roadmap and next steps
- [docs/projects/](docs/projects/) - Individual project designs:
  - `debugging-support.md` - Assembler + sim diagnostics
  - `isa-expansion.md` - Adding new instructions
  - `microcode-debugging.md` - Control path visibility
  - `microcode-compiler-improvements.md` - Compiler enhancements
  - `cartridge-tools.md` - Program tooling
- Module READMEs in each module directory

### Doxygen

Generate API documentation:
```bash
cmake --build build --target docs
# Output: build/docs/html/index.html
```

## Independent Development Workflow

When working autonomously on this project, follow this cycle for each unit of work:

### 1. Plan the Work

- Check `docs/plan.md` for current priorities and project order
- Check `docs/projects/` for detailed project designs
- Check `docs/code-review.md` and `docs/project-plan-alignment.md` for outstanding items
- Break work into small, independent steps (each step = one commit)

### 2. Implement Each Step

For each step:

```bash
# 1. Make changes (code, tests, docs)
# 2. Build and test locally
make test

# 3. Check coverage if adding new code
make coverage
# Review: build/coverage/html/index.html

# 4. Commit with descriptive message
git add -A
git commit -m "Brief description of change"

# 5. Push and monitor CI
git push
# Watch GitHub Actions for failures
```

### 3. Quality Gates

Before considering a step complete:

- [ ] All tests pass locally (`make test`)
- [ ] No new compiler warnings
- [ ] New code has test coverage
- [ ] CI passes after push
- [ ] Documentation updated if behavior changed

### 4. Update Planning Docs

After completing a project or milestone:

- Update `docs/plan.md` with new status
- Update `docs/project-plan-alignment.md` if applicable
- Mark completed items in `docs/projects/*.md`

### Step Size Guidelines

**Good step size** (one commit each):
- Add one new test file
- Implement one new class/function with tests
- Fix one bug with regression test
- Update one documentation file
- Add one new instruction to ISA + microcode + test

**Too large** (break into smaller steps):
- Implement entire new feature at once
- Multiple unrelated changes in one commit
- Large refactoring without incremental tests

### Recovery from CI Failures

If CI fails after push:

1. Read the CI log to identify the failure
2. Fix locally and verify with `make test`
3. Commit the fix with message referencing the issue
4. Push and verify CI passes

### Current Project Priorities

See `docs/plan.md` for the authoritative list. Current status as of January 2026:

**Completed Projects:**
1. ✓ **Debugging support** - Debug symbols, trace buffer, IPC register, failure diagnostics all complete
2. ✓ **Logging improvements** - Structured logging with CLI/env configuration complete
3. ✓ **Microcode compiler improvements** - All 5 validators and 3 optimizers complete, compiler restructured
4. ✓ **Microcode debug visibility** - Decoder, YAML output, CLI utility complete
5. ✓ **Sim module cleanup** - All 11 phases complete. Register hierarchy redesign, memory refactoring with factory pattern, controller submodule with hardware-ish ROM storage, TMP register with 128-bit control words, CPU singleton refactoring, RunResult improvements with HaltReason/CpuState, HDL validation, and BusValidator updates all complete

**Next Priorities:**
1. **ISA expansion** - Begin with ALU instructions batch (ADD, AND, OR, XOR). All infrastructure is now in place (debugging tools, structured logging, robust microcode validation/optimization, clean sim architecture with hardware-ish components)
