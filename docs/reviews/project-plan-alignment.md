# Project Plan Alignment Analysis

**Date:** 2026-01-13 (Updated)
**Previous Update:** 2026-01-11

This document compares the project plans in `docs/projects/` with actual implementation state and recommends next steps.

---

## Debugging Support (docs/projects/debugging-support.md)

### Status: COMPLETE ✓

All milestones achieved. Here's the final status:

| Subproject | Milestone | Status | Evidence |
|------------|-----------|--------|----------|
| **1) Assembler Debug Sidecar** | M0: Schema + validator | DONE | `assembler/debug_schema.json` exists |
| | M1: Assembler emits JSON | DONE | Tests produce `.json` files |
| **2) Simulator Debug Loader** | M0: Loader + schema validation | DONE | `debug_symbols.h/cpp`, `LoadDebugSymbols()` |
| | M1: PC → file/line lookup | DONE | `DebugSymbols::Lookup()`, `pc_to_source` map |
| **IPC Register** | Hardware-ish solution | DONE | `ipc_` in Controller (HDL + sim), `instruction_start` control |
| **3) Failure Diagnostics** | M0: Register dump | DONE | `debug_dump.cpp`, `FormatDebugDump()` |
| | M1: Source location context | DONE | Uses `instruction_source_location()` |
| **4) Trace Buffer** | M0: Last N PC/IR/state | DONE | `debug_trace.h`, `DebugTraceBuffer` |
| | M1: Symbolized locations | DONE | `FormatTraceLocation()` in dump |
| **5) Test Harness** | M0: `--debug` flag | DONE | `run.cpp` accepts `--debug debug.json` |
| | M0: `--trace-depth` flag | DONE | Configurable trace depth |
| | M1: Assert debug output | DONE | Test coverage exists in debug_dump_test.cpp |

### Project Complete

All planned functionality implemented and tested. Debug symbols, trace buffer, IPC register, and failure diagnostics all working as designed.

---

## Microcode Compiler Improvements (docs/projects/microcode-compiler-improvements.md)

### Status: COMPLETE ✓

| Subproject | Status | Implementation |
|------------|--------|---------------|
| **Validation Expansion** | | |
| - Bus write conflict validator | DONE ✓ | `bus_validator.h/cpp` |
| - Status validator | DONE ✓ | `status_validator.h/cpp` |
| - Stage validator | DONE ✓ | `stage_validator.h/cpp` |
| - Control conflict validator | DONE ✓ | `control_conflict_validator.h/cpp` |
| **Optimization Passes** | | |
| - Empty step removal | DONE ✓ | `empty_step_optimizer.h/cpp` |
| - Duplicate step removal | DONE ✓ | `duplicate_step_optimizer.h/cpp` |
| - Step merging | DONE ✓ | `step_merging_optimizer.h/cpp` |
| **Compiler Restructuring** | DONE ✓ | Preamble/validators/transformers pattern |

### Project Complete

All validators and optimizers implemented with full test coverage. Compiler restructured with defensive validation pattern.

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

## Logging Improvements (docs/projects/logging-improvements.md)

### Status: COMPLETE ✓

| Feature | Status | Implementation |
|---------|--------|---------------|
| Logging facade | DONE ✓ | `base/log.h` with IRATA2_LOG_* macros |
| Sim lifecycle events | DONE ✓ | sim.start, sim.halt, sim.crash, sim.timeout |
| Failure-path logging | DONE ✓ | Debug dump and trace on failure |
| CLI log-level config | DONE ✓ | `--log-level` flag + IRATA2_LOG_LEVEL env var |

### Project Complete

Full logging infrastructure in place with absl::log backend, structured events, and CLI configuration.

---

## Microcode Debugging (docs/projects/microcode-debugging.md)

### Status: COMPLETE ✓

| Feature | Status | Implementation |
|---------|--------|---------------|
| Microcode decoder | DONE ✓ | `microcode/debug/decoder.h` |
| Control word decoding | DONE ✓ | `DecodeControlWord()` method |
| Program dumps | DONE ✓ | `DumpProgram()`, `DumpInstruction()` |
| YAML output | DONE ✓ | `DumpProgramYaml()`, `DumpInstructionYaml()` |
| CLI utility | DONE ✓ | `microcode_dump_main.cpp` |

### Project Complete

Full microcode introspection tooling implemented. All milestones M0-M2 achieved.

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

Based on current completion status:

### 1. ISA Expansion - ALU Batch (Highest Priority)

With all debugging and validation infrastructure complete, begin ISA growth:
- Implement ADD, AND, OR, XOR instructions
- Add ASM integration tests for each
- Validates ALU wiring works beyond just SUB
- Microcode validators will catch issues early

### 2. Continue Sim Module Cleanup (Interleaved)

Complete remaining phases from cleanup_plan.md as needed:
- Phase 3: Register hierarchy redesign
- Phase 5: Memory refactoring
- Phase 6: Controller submodule (largest effort)
- Phase 7: CPU constructor refactoring
- Phase 8: HDL enforcement

These are architectural improvements that can be done incrementally.

### 3. ISA Expansion - Further Batches

Continue vertical growth:
- Stack operations (needs stack pointer)
- More addressing modes
- Branch/subroutine instructions
- Indexed addressing

### 4. Cartridge Tools (Optional)

Only if needed for workflow improvement:
- Disassembler
- Round-trip testing
- Binary inspection tools

---

## Summary Table

| Project | Status | Priority | Next Action |
|---------|--------|----------|-------------|
| Debugging Support | **Complete** ✓ | - | Closed |
| Logging Improvements | **Complete** ✓ | - | Closed |
| Microcode Compiler | **Complete** ✓ | - | Closed |
| Microcode Debugging | **Complete** ✓ | - | Closed |
| Sim Module Cleanup | In Progress (50%) | MEDIUM | Continue phases 3-8 |
| ISA Expansion | Not started | **HIGH** | Begin ALU batch |
| Cartridge Tools | Partial | LOW | Defer |
