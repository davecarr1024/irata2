# IRATA2 Vertical Slice Plan

This plan targets a fast end-to-end slice: `.asm` tests assemble to cartridges, run in the simulator with compiled microcode, and pass on `HLT` / fail on `CRS`.

## Goals

- End-to-end, self-hosted integration tests from `.asm` → cartridge → simulator → halt/crash.
- Minimal, working CPU as quickly as possible; add features vertically after the slice works.
- Clear module boundaries: ISA defines opcodes, microcode defines behavior, HDL defines structure, sim runs it.

## Scope of the Vertical Slice

- ISA: minimal `HLT`, `NOP`, `CRS` instruction set.
- Microcode: YAML → IR codegen, compiler pipeline, ROM encoding, ISA coverage validation.
- HDL: structural CPU with controller + IR + SC, halt/crash controls, path resolution.
- Sim: runtime state for registers, buses, memory; control decoding and status wiring.
- Assembler: pipeline that consumes ISA definitions and emits cartridge + debug info.
- Tests: `tests/*.asm` auto-discovered and run end-to-end.

## Phases

### Phase 0: Vertical Slice Harness

Deliverables:
- `tests/` directory with `.asm` programs and metadata for expected outcomes.
- Assembler CLI that outputs a cartridge binary + debug info.
- Test runner (C++ or Python) that:
  - Discovers `.asm` files.
  - Assembles to cartridges.
  - Runs the simulator until `HLT` or `CRS`.
  - Reports pass/fail based on expected halt/crash.

Notes:
- Prefer adding CMake wiring so `ctest` discovers and runs these integration tests.
- Keep the harness stable so it can be reused for later features.
- Default `.org` is `0x8000` for all programs until directives are supported.
- Cartridge ROM image is `0x0000-0x7FFF` and is mapped to CPU `0x8000-0xFFFF` by the sim memory module.

### Phase 1: Microcode System Completion

Deliverables:
- `microcode/microcode.yaml` + `generate_microcode.py` → generated IR C++.
- `ir::Builder` used by generated IR for fast-fail control resolution.
- Compiler pipeline wiring (Fetch/Sequence transformers + validators in MVP).
- ROM encoding format + lookup mechanism (opcode/step/status → control word).

Notes:
- Status wiring can be stubbed for MVP (no branches yet).
- ISA coverage validation should be run after the IR is built.

### Phase 2: Simulator Core

Deliverables:
- Runtime bus and register models aligned with HDL paths.
- Controller component that fetches/decodes instruction bytes using IR + SC.
- Microcode ROM decoding and control assertion each tick.
- Memory model with MAR + read/write controls sufficient for fetch/decode.
- Status register (SR) with 6502-style flags; push/pop wiring is planned.
- Halt/crash signals terminate execution.

Notes:
- Enforce single-writer-per-bus at runtime or via microcode validation.
- Unmapped memory reads return `0xFF` (CRS opcode) and should hard-fail tests.

### Phase 3: Assembler MVP

Deliverables:
- Assembler pipeline that:
  - Loads ISA definition.
  - Parses `.asm` into an AST.
  - Resolves labels and addressing (only implied for MVP).
  - Emits a cartridge binary and debug info (symbol map).

Notes:
- Start with implied-only instruction encoding (HLT/NOP/CRS), then extend addressing modes.

### Phase 4: Integration Tests

Deliverables:
- `.asm` programs for positive halt paths and negative crash paths.
- Expected exit conditions (HLT/CRS) tracked per test.
- CI integration so tests run with `ctest`.

## Proposed Test Harness Structure

```
/tests
  hlt_smoke.asm
  nop_then_hlt.asm
  crash_smoke.asm
  crash_expected.meta.json  # optional expected outcome metadata
```

- If metadata is a concern, embed expected outcome as a comment (e.g., `; EXPECT: CRS`).
- The runner should default to `HLT` unless explicitly marked as `CRS`.

## Milestones and Definitions of Done

- **M0**: `.asm` discovery + test runner works, even if the simulator is stubbed.
- **M1**: Microcode YAML → ROM encoded; sim executes HLT/NOP/CRS program correctly.
- **M2**: Tests run in CI: HLT tests pass, CRS tests fail unless marked expected.

## Risks / Unknowns

- ROM encoding format and control bit allocation need to be finalized.
- Status wiring strategy is pending (status controls + status validation).
- Assembler interface (CLI + cartridge format) needs consensus.
  - Cartridge should be a fixed, minimal header + ROM image; debug symbols live in a sidecar file.
  - ControlEncoder should use HDL traversal order to map controls to bits.

## Ownership Boundaries

- **ISA**: instruction definitions + encoding tables.
- **Microcode**: behavior sequences and compilation to ROM.
- **HDL**: structural paths and control definitions.
- **Sim**: runtime state, control assertion, program execution.
- **Assembler**: `.asm` → cartridge + debug info.
