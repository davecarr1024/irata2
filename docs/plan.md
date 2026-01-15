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
- **Sim module cleanup in progress**: Phases 1, 2, 4, 9, 10 of cleanup_plan.md complete. Component hierarchy improvements, control hierarchy redesign, ComponentWithBus abstraction, directory/namespace organization done. Remaining phases (3, 5, 6, 7, 8) documented in cleanup_plan.md.

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
5. **Sim module cleanup** - Architectural refactoring per `cleanup_plan.md`. This is a
   long-running effort that can be interleaved with other work. Phases 1, 2, 4, 9, 10
   complete. Remaining phases (3, 5, 6, 7, 8) provide foundation for advanced features.
6. Program tooling and cartridge inspection for safer workflows - **Optional/Deferred**.
7. ISA expansion in batches - **Next priority** now that debugging/validation tooling is complete.

## Next Overall Steps

With debugging support, logging, and microcode compiler improvements complete, the codebase is well-positioned for growth:

1. **Continue sim module cleanup** - Complete remaining phases (3, 5, 6, 7, 8) of cleanup_plan.md as needed. These provide architectural improvements but are not blocking.

2. **Begin ISA expansion** - Start with ALU instructions batch (ADD, AND, OR, XOR) per `docs/projects/isa-expansion.md`. The validation and debugging infrastructure is now in place to support safe ISA growth.

3. **Maintain test coverage** - Add ASM integration tests for each new instruction, ensuring end-to-end validation through the full toolchain.

4. **Monitor microcode complexity** - Use the microcode decoder and validators to ensure control sequences remain maintainable as the ISA expands.
