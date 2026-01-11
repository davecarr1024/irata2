# IRATA2 Next Steps

This document is a top-level index of upcoming projects and near-term
direction. Individual projects live in `docs/projects/`.

## Current Status

- The MVP vertical slice is in place (assembler -> cartridge -> sim -> tests).
- LDA/CMP/JEQ and ALU wiring are implemented.
- Status analyzer and status bit controls exist.
- ASM integration tests are split by instruction.
- **Debugging support complete**: debug symbols, trace buffer, IPC, dump output, and test harness integration finished.

## Active Project Ideas

- Debugging support in assembler + sim: see `docs/projects/debugging-support.md` (complete)
- ISA expansion in batches: see `docs/projects/isa-expansion.md`
- Microcode debug visibility: see `docs/projects/microcode-debugging.md`
- Microcode compiler improvements: see `docs/projects/microcode-compiler-improvements.md`
- Program tooling and cartridge inspection: see `docs/projects/cartridge-tools.md`

## Planned Project Order (Defensive)

1. ~~Debugging support (assembler + sim)~~ - **Complete**.
2. Microcode compiler improvements (validators, optimizers) to harden correctness early.
3. Microcode debug visibility for control path transparency.
4. Program tooling and cartridge inspection for safer workflows.
5. ISA expansion in batches - **Deferred** until tooling supports safe vertical growth.

## Next Overall Steps

1. Pick one project in `docs/projects/` and drive it to a minimal MVP.
2. Add small, focused tests (unit + ASM integration) per milestone.
3. Keep microcode/HDL/sim structure aligned as the ISA grows.
