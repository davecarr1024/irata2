# IRATA2 Next Steps

## Snapshot

- ISA has 152 instructions across 12 addressing modes
- Assembler, microcode compiler, and simulator are in place
- Debug tooling is present: debug symbols, trace buffer, debug dump, IPC register, microcode decoder/dump
- Microcode validation/optimization pipeline exists but has gaps (see Urgent below)

**Known Issues:** 4 tests failing (asm_jsr, asm_php, asm_plp, asm_rts) - stack operations have microcode bugs.

## Urgent: Bus Validator Rewrite

The bus validator (`microcode/src/compiler/bus_validator.cpp`) uses fragile name-based string matching to detect bus conflicts. This approach broke when new controls were added and is now failing to catch real conflicts.

**Problem:** Hardcoded component names and pattern matching that doesn't scale and misses conflicts.

**Solution:** Rewrite to use HDL traversal instead of string matching:
1. Remove all name-based lookups - Delete `AnalyzeControl()` and its hardcoded lists
2. Traverse HDL tree to discover all controls automatically
3. Use control metadata (ReadControl vs WriteControl, which bus)
4. Build map from ControlInfo* to (bus, read/write) at construction
5. Validate using map lookup

**Files:** `microcode/src/compiler/bus_validator.cpp`, `bus_validator.h`, `bus_validator_test.cpp`

## Broken Tests (Blocked on Bus Validator)

| Test | Issue |
|------|-------|
| `asm_php` | PLP doesn't restore status - wrong control direction |
| `asm_plp` | Same as php |
| `asm_jsr` | Jumps to stack address - MAR overwritten before copy to PC |
| `asm_rts` | Depends on JSR pushing correct return address |

## Goal: Interactive Demos

**Target:** Run small, interactive programs with input, frame timing, and visible output.

### Next Steps

1. **Define the demo surface**
   - Decide a minimal video interface (framebuffer or tile map) and input model
   - Choose addresses and register semantics for memory-mapped I/O
   - Decide output size and refresh cadence for the demo runner

2. **Add memory-mapped I/O hardware**
   - HDL: add video buffer and input registers on the data bus
   - Sim: implement backing storage and input injection
   - Expose minimal controls for bus access and debug visibility

3. **Build the demo runner**
   - CLI: `irata2_run --rom demo.cartridge --fps 60 --input <device>`
   - Frame loop: run N cycles per frame, present buffer, collect input
   - On crash/timeout: emit debug dump + trace for quick diagnosis

4. **Author demo programs**
   - Start with a blinking pixel, then a sprite, then input-driven movement
   - Provide a tiny demo suite in `assembler/test/asm_tests/` or a new `demos/` folder
   - Document the I/O map for demo authors

## Debug Tooling Backlog

These were documented previously and still remain useful for diagnosis and demo workflows:

- Cartridge inspection CLI (header dump, validation, quick ROM summary)
- Disassembler with symbol-aware output (uses debug sidecar when present)
- Round-trip assemble/disassemble tests for regression coverage
- Optional program metadata viewer (build info, entry point, symbols)

See `docs/projects/cartridge-tools.md` for details.

## Performance Benchmark Plan

**Goal:** Track simulator throughput without adding flaky perf assertions to unit tests.

1. Add a standalone benchmark target (not part of `ctest`)
   - Example: `sim/bench/bench_main.cpp` and `make bench`
2. Use deterministic workloads
   - Tight loop ROM (ALU + branches)
   - Memory-heavy ROM (loads/stores)
3. Measure and report
   - Cycles/sec, instructions/sec, and total wall time
   - Output to JSON/CSV for easy plotting
4. Document how to run
   - `./build/sim/irata2_bench --workload loop --cycles 5_000_000`
5. Track results in `docs/benchmarks.md`
6. Optional: nightly CI perf job (non-blocking)
