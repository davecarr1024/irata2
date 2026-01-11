# Project Plan Alignment Analysis

**Date:** 2026-01-11

This document compares the project plans in `docs/projects/` with actual implementation state and recommends next steps.

---

## Debugging Support (docs/projects/debugging-support.md)

### Status: ~80% Complete

This project is well underway. Here's the milestone status:

| Subproject | Milestone | Status | Evidence |
|------------|-----------|--------|----------|
| **1) Assembler Debug Sidecar** | M0: Schema + validator | DONE | `assembler/debug_schema.json` exists |
| | M1: Assembler emits JSON | DONE | Tests produce `.json` files |
| **2) Simulator Debug Loader** | M0: Loader + schema validation | DONE | `debug_symbols.h/cpp`, `LoadDebugSymbols()` |
| | M1: PC → file/line lookup | DONE | `DebugSymbols::Lookup()`, `pc_to_source` map |
| **IPC Register** | Hardware-ish solution | DONE | `ipc_` in Controller (HDL + sim), `ipc_latch` control |
| **3) Failure Diagnostics** | M0: Register dump | DONE | `debug_dump.cpp`, `FormatDebugDump()` |
| | M1: Source location context | DONE | Uses `instruction_source_location()` |
| **4) Trace Buffer** | M0: Last N PC/IR/state | DONE | `debug_trace.h`, `DebugTraceBuffer` |
| | M1: Symbolized locations | DONE | `FormatTraceLocation()` in dump |
| **5) Test Harness** | M0: `--debug` flag | DONE | `run.cpp` accepts `--debug debug.json` |
| | M0: `--trace-depth` flag | DONE | Configurable trace depth |
| | M1: Assert debug output | NOT DONE | Tests don't verify debug output format |

### Remaining Work

1. **Test harness M1**: Add tests that verify debug output is produced on failure paths
2. **Documentation**: Update README to document debug flags
3. **CI integration**: Consider enabling debug symbols in CI test runs

### Recommended Next Steps

The debugging-support project is functionally complete. Consider:
- Closing this project and noting M1 of test harness as "optional enhancement"
- OR adding a simple test that verifies `FormatDebugDump()` output format

---

## Microcode Compiler Improvements (docs/projects/microcode-compiler-improvements.md)

### Status: Partially Designed, Not Started

| Subproject | Status | Notes |
|------------|--------|-------|
| **Validation Expansion** | | |
| - Bus write conflict validator | NOT DONE | Listed in microcode/README.md as "planned" |
| - Read/write ordering validator | NOT DONE | |
| - Control conflict validator | NOT DONE | |
| **Optimization Passes** | | |
| - Dead step removal | NOT DONE | Listed as "planned" |
| - Step merging | NOT DONE | Listed as "planned" |
| **Diagnostics** | | |
| - Structured error output | PARTIAL | Errors exist but are terse |
| - Compile summary | NOT DONE | |

### Alignment with Code Review

The code review noted (M1, M2) that microcode validators are listed as "MVP" in README but not implemented. This project plan provides the roadmap for that work.

### Recommended Priority

**Medium** - Current validators (Fetch, Sequence, IsaCoverage) are sufficient for MVP. Bus validator would catch real bugs but isn't blocking.

---

## ISA Expansion (docs/projects/isa-expansion.md)

### Status: Not Started

The MVP ISA is intentionally minimal (6 instructions). This project defines the expansion roadmap.

| Batch | Status | Dependencies |
|-------|--------|--------------|
| ALU instructions | NOT DONE | ALU HDL/sim exists; needs microcode + tests |
| Register transfer | NOT DONE | |
| Stack | NOT DONE | Needs stack pointer register |
| Subroutines/branches | NOT DONE | Needs JSR/RTS microcode |
| Addressing modes | NOT DONE | Needs MAR refactoring? |

### Alignment with Code Review

- ALU has opcode bit controls in HDL/sim (4 bits for 16 operations)
- Only SUB (opcode 0x2) is implemented for CMP
- Adding ALU instructions requires: ISA YAML entries + microcode YAML entries + tests

### Recommended Priority

**High after debugging is closed** - This is the natural next step for feature growth.

---

## Microcode Debugging (docs/projects/microcode-debugging.md)

### Status: Not Started

| Feature | Status |
|---------|--------|
| Microcode decompiler | NOT DONE |
| Control word decoding | NOT DONE |
| Diff tooling | NOT DONE |
| CLI dump | NOT DONE |

### Alignment

The microcode encoders exist (`ControlEncoder::Decode()`, etc.) but no CLI or human-readable dump is implemented.

### Recommended Priority

**Low** - Nice to have for debugging microcode issues, but not blocking.

---

## Cartridge Tools (docs/projects/cartridge-tools.md)

### Status: Partially Done (via assembler)

| Feature | Status | Notes |
|---------|--------|-------|
| Cartridge inspection | PARTIAL | Header parsing in `cartridge.cpp` |
| Disassembler | NOT DONE | |
| Round-trip tests | NOT DONE | |

### Alignment

- Cartridge format has a header (answered open question)
- Debug JSON provides symbol-aware disassembly data, but no standalone disassembler

### Recommended Priority

**Low** - Assembler + debug JSON provides most value already.

---

## Recommended Project Order

Based on the code review findings and current implementation state:

### 1. Close Debugging Support (Current)

Remaining work is minimal:
- Either mark M1 test harness as "optional" and close
- Or add one test verifying debug dump format

### 2. ISA Expansion - ALU Batch

Natural next step:
- ADD, AND, OR, XOR in microcode.yaml
- Tests for each instruction
- Validates ALU wiring works beyond just SUB

### 3. Microcode Compiler Improvements - BusValidator

Adds safety:
- Catches bus contention at compile time
- Prevents subtle runtime bugs as ISA grows

### 4. ISA Expansion - Further Batches

Continue as needed:
- Stack operations
- More addressing modes
- Subroutines

### 5. Microcode Debugging / Cartridge Tools

Only if needed:
- Useful for debugging complex microcode issues
- Lower priority for MVP

---

## Summary Table

| Project | Status | Priority | Next Action |
|---------|--------|----------|-------------|
| Debugging Support | ~80% | HIGH | Close or add M1 test |
| Microcode Compiler | Not started | MEDIUM | Add BusValidator |
| ISA Expansion | Not started | HIGH | ALU batch first |
| Microcode Debugging | Not started | LOW | Defer |
| Cartridge Tools | Partial | LOW | Defer |
