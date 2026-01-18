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
- **Sim module cleanup complete** ✓: All 11 phases of cleanup_plan.md finished (100%). Register hierarchy redesign, memory refactoring with factory pattern, controller submodule with hardware-ish ROM storage, TMP register with 128-bit control words, CPU singleton refactoring, RunResult improvements with HaltReason/CpuState, HDL validation, and BusValidator updates all complete. The sim module now has clean architecture with proper separation of concerns, hardware-ish components, and comprehensive validation.
- **ISA expansion in progress**: Phases 1-7 complete (ALU core, immediate ops, register transfer, zero page, absolute addressing, inc/dec, conditional branches). Next up: indexed addressing per `docs/projects/isa-expansion-plan.md`.

## Active Project Ideas

- ~~Debugging support in assembler + sim~~ - **Complete** ✓ - see `docs/projects/debugging-support.md`
- ~~Logging improvements for sim debugging~~ - **Complete** ✓ - see `docs/projects/logging-improvements.md`
- ~~Microcode debug visibility~~ - **Complete** ✓ - see `docs/projects/microcode-debugging.md`
- ~~Microcode compiler improvements~~ - **Complete** ✓ - see `docs/projects/microcode-compiler-improvements.md`
- ~~Sim module cleanup~~ - **Complete** ✓ - see `docs/projects/cleanup.md` and `docs/projects/cleanup_plan.md`
- **ISA expansion** (next priority) - see `docs/projects/isa-expansion.md`
- Program tooling and cartridge inspection (optional/deferred) - see `docs/projects/cartridge-tools.md`

## Planned Project Order (Defensive)

This ordering is intentionally conservative to maximize stability and reduce
risk as the ISA grows.

1. ~~Debugging support (assembler + sim)~~ - **Complete** ✓
2. ~~Logging improvements for sim debugging~~ - **Complete** ✓
3. ~~Microcode compiler improvements (validators, optimizers)~~ - **Complete** ✓
4. ~~Microcode debug visibility for control path transparency~~ - **Complete** ✓
5. ~~Sim module cleanup~~ - **Complete** ✓ - All 11 phases of `cleanup_plan.md` finished.
   Register hierarchy, memory factory pattern, hardware-ish controller with ROM storage,
   TMP register with 128-bit control words, CPU singleton refactoring, RunResult improvements,
   HDL validation, and BusValidator updates all complete.
6. Program tooling and cartridge inspection for safer workflows - **Optional/Deferred**.
7. **ISA expansion in batches** - **Next priority**. All infrastructure complete.

## Next Overall Steps

With all foundational projects complete (debugging support, logging, microcode compiler improvements, and sim cleanup), the codebase is fully prepared for ISA expansion:

1. **Continue ISA expansion** - Next batch is indexed addressing modes (Phase 8) per `docs/projects/isa-expansion.md`. All infrastructure is in place:
   - Comprehensive debugging tools (symbols, trace, IPC, dump output)
   - Structured logging with failure diagnostics
   - Robust microcode validation and optimization
   - Clean sim architecture with hardware-ish components
   - TMP register for complex addressing modes
   - 128-bit control words for future growth
   - HDL validation for structural consistency

2. **Maintain test coverage** - Add ASM integration tests for each new instruction, ensuring end-to-end validation through the full toolchain.

3. **Monitor microcode complexity** - Use the microcode decoder and validators to ensure control sequences remain maintainable as the ISA expands.

4. **Leverage the infrastructure** - The complete tooling suite (validators, optimizers, debuggers, tracers) enables confident, incremental ISA growth with immediate feedback on errors.
