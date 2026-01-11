# IRATA2 - 8-bit CPU Simulator

[![CI](https://github.com/davecarr1024/irata2/actions/workflows/ci.yml/badge.svg)](https://github.com/davecarr1024/irata2/actions/workflows/ci.yml)
[![codecov](https://codecov.io/gh/davecarr1024/irata2/branch/main/graph/badge.svg)](https://codecov.io/gh/davecarr1024/irata2)

A cycle-accurate 8-bit CPU simulator in C++ that feels like building a breadboard computer.

## Vision

Build a simulator where components compose like hardware modules. The system is testable, incremental, and provably correct at every step. The long-term dream: a vector graphics Asteroids game running on a 2600-style system with memory-mapped I/O, written entirely in assembly.

## Architecture

The project is organized into independent, self-contained modules:

| Module | Description | Documentation |
|--------|-------------|---------------|
| [base](base/) | Fundamental types (`Byte`, `Word`, `TickPhase`) | [base/README.md](base/README.md) |
| [hdl](hdl/) | Hardware Definition Language - immutable structural metadata | [hdl/README.md](hdl/README.md) |
| [isa](isa/) | Instruction Set Architecture definitions (YAML + code generation) | [isa/README.md](isa/README.md) |
| [sim](sim/) | Runtime simulator with mutable state | [sim/README.md](sim/README.md) |
| [microcode](microcode/) | Microcode DSL, IR, compiler, encoder | [microcode/README.md](microcode/README.md) |

**Build order** (managed automatically by CMake):
1. `base` - No dependencies
2. `hdl` - Depends on `base`
3. `isa` - Depends on `base`
4. `sim` - Depends on `base`, `hdl`
5. `microcode` - Depends on `base`, `hdl`, `isa`

## Current Status

- `base` is implemented and provides `Byte`, `Word`, and `TickPhase`.
- `hdl` is implemented with immutable components, buses, controls, and registers, including controller/IR/SC and halt/crash controls.
- `isa` is reduced to the minimal MVP instruction set (HLT/NOP/CRS).
- `microcode` has MVP IR + compiler passes/validators, plus control/status encoding into microcode programs; YAML codegen and ROM image emission are next.
- `sim` implements runtime components with control assertion from compiled microcode programs; fetch/decode wiring and instruction execution are next.

See [docs/plan.md](docs/plan.md) for the vertical-slice roadmap.

## Design Philosophy

- **Hardware-ish**: Components compose like hardware modules with buses, registers, and controls
- **Strongly Typed**: Invalid states are unrepresentable in the type system
- **Compile-Time Safety**: CRTP templates for zero-cost abstractions and compile-time verification
- **Immutable Structure**: HDL describes structure; simulation adds runtime state
- **Testable**: 100% test coverage goal with GoogleTest

## Roadmap Notes

- The vertical slice plan targets end-to-end `.asm` integration tests; see `plan.md`.
- The design doc references an assembler module; it is not yet present in this repo.

## Quick Start

```bash
# Build with tests
cmake -B build -DBUILD_TESTING=ON
cmake --build build

# Run tests
ctest --test-dir build --output-on-failure
```

## Building

See [Building](#building-options) for all build options including code coverage and release builds.

## Coverage

```bash
cmake -B build -DENABLE_COVERAGE=ON -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
./scripts/coverage.sh build
```

Coverage report output: `build/coverage/html/index.html`

### Building Options

```bash
# Standard build
cmake -B build
cmake --build build

# With tests
cmake -B build -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure

# With code coverage
cmake -B build -DENABLE_COVERAGE=ON -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build
./scripts/coverage.sh build

# Release build
cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF
cmake --build build
```

## Documentation

- [docs/design.md](docs/design.md) - Detailed architecture and design decisions
- [docs/plan.md](docs/plan.md) - Vertical slice roadmap
- [docs/code-review.md](docs/code-review.md) - Code review report and fix plan
- Module-specific documentation in each module's README
