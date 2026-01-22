# IRATA2 Next Steps

## Snapshot

- ISA is complete: 152 instructions across 12 addressing modes
- Assembler, microcode compiler, and simulator are in place with full test coverage
- Debug tooling is present: debug symbols, trace buffer, debug dump, IPC register, microcode decoder/dump
- Microcode validation/optimization pipeline is stable

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
   - `./build/bench --rom bench_loop.cartridge --cycles 5_000_000`
5. Optional: nightly CI perf job (non-blocking)
