# IRATA2 Next Steps

## Snapshot

- ISA has 152 instructions across 12 addressing modes (all tests passing)
- Assembler, microcode compiler, and simulator are in place with full test coverage
- Debug tooling is present: debug symbols, trace buffer, debug dump, IPC register, microcode decoder/dump
- Microcode validation/optimization pipeline is stable

## Goal: Interactive Demos

**Target:** Run small, interactive programs with input, frame timing, and visible output.

### Next Steps

**See `docs/projects/demo-surface.md` for complete specification.**

1. **✓ Define the demo surface**
   - ✓ Vector graphics coprocessor (VGC) with streaming MMIO interface at $4100
   - ✓ Input device with keyboard queue at $4000
   - ✓ 2-bit monochrome green intensity (arcade vector aesthetic)
   - ✓ 256x256 logical resolution, 30 FPS default (3,333 cycles/frame @ 100 KHz)
   - ✓ Swappable backends: ImageBackend (testing) + SdlBackend (display)
   - ✓ Sound device deferred but documented ($4200, square wave)
   - ✓ MMIO region: $4000-$7FFF (between RAM at $0000 and ROM at $8000)

2. **Phase 1: Input Device (MVP)** - *DONE*
   - ✓ Implement `sim/io/input_device.{h,cpp}` with 16-byte queue and MMIO registers
   - ✓ Add MMIO routing in memory map ($4000-$400F)
   - ✓ Write unit tests for queue behavior, register reads/writes
   - ✓ Write integration test: assembly program reads input, stores in RAM
   - **Deliverable:** Keyboard input flows to CPU via polling

3. **Phase 2: Vector Graphics Coprocessor (MVP)** - *DONE*
   - ✓ Implement VgcBackend interface (clear, draw_point, draw_line, present)
   - ✓ Implement ImageBackend for headless testing (256x256 framebuffer)
   - ✓ Implement VectorGraphicsCoprocessor with streaming registers
   - ✓ Add MMIO routing for VGC region ($4100-$4108) via extra region factories
   - ✓ Unit tests + integration test (assembly draws shapes, verify framebuffer pixels)
   - **Deliverable:** Graphics testing loop closed (Assembly → MMIO → VGC → ImageBackend → verification)

4. **Phase 3: SDL Frontend (Basic Window)** - *DONE*
   - ✓ Implement SdlBackend rendering to SDL_Renderer with green CRT aesthetic
   - ✓ Add `frontend/` module with SDL2 dependency (optional build flag)
   - ✓ Implement DemoRunner class (window, frame loop, input handling)
   - ✓ CLI: `irata2_demo --rom game.cartridge --fps 30 --scale 2 --cycles-per-frame 3333`
   - **Deliverable:** Window opens, renders VGC commands, accepts keyboard input

5. **Phase 4-7: Demo Programs**
   - ✓ Phase 4: Blinking pixel demo (simple animation)
   - ✓ Phase 5: Moving sprite with arrow key control
   - Phase 6: IRQ support (interrupt-driven input, deferred)
   - ✓ Phase 7: Asteroids prototype (ship, asteroids, shooting, collision)

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
