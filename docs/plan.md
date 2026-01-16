# IRATA2 Next Steps

This document is a top-level index of upcoming projects and near-term
direction. Individual projects live in `docs/projects/`.

## Current Status

- The MVP vertical slice is in place (assembler -> cartridge -> sim -> tests).
- LDA/CMP/JEQ and ALU wiring are implemented.
- Status analyzer and status bit controls exist.
- ASM integration tests are split by instruction.
- **Debugging support complete** ✓: debug symbols, trace buffer, IPC, dump output, and test harness integration finished.
- **Logging improvements complete** ✓: structured logging with lifecycle events, failure-path dumps, CLI/env configuration, and comprehensive documentation.
- **Microcode compiler improvements complete** ✓: all 5 validators (BusValidator, StatusValidator, StageValidator, ControlConflictValidator, SequenceValidator), 3 optimizers (EmptyStep/DuplicateStep/StepMerging), compiler restructuring with preamble/validators/transformers pattern.
- **Microcode debug visibility complete** ✓: decoder, YAML output, CLI utility (microcode_dump_main), all milestones M0-M2.
- **Sim module cleanup in progress**: Phases 1, 2, 3 (including 3.5), 4, 5, 6, 7.1, 9, 10 of cleanup_plan.md complete (80% done). Register hierarchy redesign, memory refactoring with factory pattern, controller submodule with hardware-ish ROM storage, TMP register with 128-bit control words, and CPU singleton refactoring all finished. InstructionMemory now "burns" microcode into RomStorage at initialization. Remaining phases (7.2, 8, 10.1) are optional enhancements, not critical path.

## Active Project Ideas

- ~~Debugging support in assembler + sim~~ - **Complete** ✓ - see `docs/projects/debugging-support.md`
- ~~Logging improvements for sim debugging~~ - **Complete** ✓ - see `docs/projects/logging-improvements.md`
- ~~Microcode debug visibility~~ - **Complete** ✓ - see `docs/projects/microcode-debugging.md`
- ~~Microcode compiler improvements~~ - **Complete** ✓ - see `docs/projects/microcode-compiler-improvements.md`
- **Sim module cleanup** (in progress) - see `docs/projects/cleanup.md` (principles) and `docs/projects/cleanup_plan.md` (implementation plan)
- **ISA expansion** (next priority) - see `docs/projects/isa-expansion.md`
- Program tooling and cartridge inspection (optional/deferred) - see `docs/projects/cartridge-tools.md`

## Planned Project Order (Defensive)

This ordering is intentionally conservative to maximize stability and reduce
risk as the ISA grows.

1. ~~Debugging support (assembler + sim)~~ - **Complete** ✓
2. ~~Logging improvements for sim debugging~~ - **Complete** ✓
3. ~~Microcode compiler improvements (validators, optimizers)~~ - **Complete** ✓
4. ~~Microcode debug visibility for control path transparency~~ - **Complete** ✓
5. **Sim module cleanup** - Architectural refactoring per `cleanup_plan.md`. **80% complete** ✓
   Phases 1, 2, 3 (including 3.5 TMP register), 4, 5, 6, 7.1 (singleton refactoring), 9, 10
   complete. Remaining phases (7.2 RunResult improvements, 8 HDL enforcement, 10.1 BusValidator)
   are optional enhancements that can be done after ISA expansion.
6. Program tooling and cartridge inspection for safer workflows - **Optional/Deferred**.
7. ISA expansion in batches - **Next priority** now that debugging/validation tooling is complete.

## Next Overall Steps

With debugging support, logging, microcode compiler improvements, and 80% of sim cleanup complete (including TMP register, 128-bit control words, and singleton refactoring), the codebase is well-positioned for ISA expansion:

1. **Begin ISA expansion** - Start with ALU instructions batch (ADD, AND, OR, XOR) per `docs/projects/isa-expansion.md`. The validation infrastructure, debugging tools, hardware-ish controller with ROM storage, and expanded control word capacity (128-bit) are now in place to support safe ISA growth.

2. **Maintain test coverage** - Add ASM integration tests for each new instruction, ensuring end-to-end validation through the full toolchain.

3. **Monitor microcode complexity** - Use the microcode decoder and validators to ensure control sequences remain maintainable as the ISA expands.

4. **Optional: Complete remaining sim cleanup** - Phases 7.2 (RunResult improvements), 8 (HDL enforcement), and 10.1 (BusValidator enhancements) provide additional polish but are not blocking ISA expansion. These can be revisited after initial ISA batches if desired.
