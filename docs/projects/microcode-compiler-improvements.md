# Microcode Compiler Improvements

**Status: COMPLETE** ✓

All validators (BusValidator, StatusValidator, StageValidator, ControlConflictValidator), all optimizers (EmptyStep, DuplicateStep, StepMerging), and compiler restructuring complete.

## Goals

- Improve correctness with stronger validators.
- Reduce compiled microcode size and redundancy.
- Surface clearer errors and diagnostics for microcode authors.

## Scope

- New validators (bus usage, conflicting writes, dead control asserts).
- Optimizers (dead step removal, step merging, canonicalization).
- Compiler diagnostics (structured error output, source mapping hooks).

## Reference Pipeline (pirata)

The pirata compiler runs a multi-pass pipeline with validation between each
transform. These are the stages to mirror for parity:

**Transforms**
- FetchTransformer: injects the uniform fetch preamble.
- SequenceTransformer: injects sequence counter increment/reset controls.
- EmptyStepOptimizer: removes steps that assert no controls.
- DuplicateStepOptimizer: removes consecutive steps with identical controls.
- Optimizer: merges adjacent compatible steps within a stage when phases allow.

**Validators**
- FetchValidator: ensures fetch preamble shape and controls are correct.
- SequenceValidator: enforces correct sequence counter behavior.
- BusValidator: enforces single-writer-per-bus and valid bus access patterns.
- StatusValidator: ensures complete status coverage for conditional variants.
- StageValidator: enforces stage numbering and monotonic step order.
- InstructionCoverageValidator: ISA completeness and duplicate checks.

The target IRATA2 pipeline should keep this ordering:
1) FetchTransformer
2) SequenceTransformer
3) Validators (Fetch, Sequence, Bus, Status, Stage, Coverage)
4) EmptyStepOptimizer → validate
5) DuplicateStepOptimizer → validate
6) Optimizer → validate

## Current Pipeline (IRATA2)

- FetchTransformer → FetchValidator
- SequenceTransformer
- IsaCoverageValidator
- SequenceValidator
- Encode control/status words, emit microcode table

Gaps to close: Bus/Status/Stage validators, optimizer passes, and validation
between each transform.

## Proposed Work

## Decisions

- Optimizer passes are **on by default**; run the full validator suite after
  each optimizer pass.
- Status variants are **single-bit only**; if any variant specifies a status,
  the opposite status variant must also be defined.
- Bus validation enforces: 0/1 writers per bus per tick, readers require a
  writer, writers require at least one reader, and parallel operations across
  distinct buses are allowed.
- Stage numbering must be monotonic 0..n; provide a repair transformer that
  flattens stages, followed by a confirming validator.

### 1) Validation Expansion
**Design details**
- **BusValidator** (pirata parity): for each step, build a bus->writer map and
  error on multiple writers; require that any readers imply a writer and that
  any writer implies at least one reader; allow concurrent operations on
  distinct buses.
- **StatusValidator** (pirata parity): expand partial status variants and
  verify exact coverage for each instruction; fail on overlaps or gaps. Enforce
  single-status conditions per variant and require an explicit opposite variant
  when any status is specified.
- **StageValidator** (pirata parity): ensure steps start at stage 0 and are
  monotonically increasing; reject duplicates or gaps when not expected.
- **Control conflict validator** (IRATA2-specific): detect mutually exclusive
  controls (e.g., read+write same register, multiple ALU op selects).
- **Read/write ordering validator** (IRATA2-specific): enforce required
  phase ordering between controls (write before read, process after read).

**Milestones**
- M0: BusValidator + tests.
- M1: StatusValidator + tests.
- M2: StageValidator + tests.
- M3: Read/write ordering validator + tests.
- M4: Control conflict validator + tests.

### 2) Optimization Passes
**Design details**
- **EmptyStepOptimizer** (pirata parity): remove steps with zero asserted
  controls, preserving stage boundaries.
- **DuplicateStepOptimizer** (pirata parity): collapse adjacent steps that have
  identical control sets within the same stage.
- **Optimizer** (pirata parity): merge adjacent steps when phase bounds do not
  conflict and no bus contention is introduced by the merge.
- **Control word canonicalization** (IRATA2-specific): deduplicate identical
  control words after encoding to reduce ROM usage.

**Milestones**
- M0: EmptyStepOptimizer + tests.
- M1: DuplicateStepOptimizer + tests.
- M2: Optimizer + tests (phase-aware merge).
- M3: Control word canonicalization (optional, post-encoding).

### 2a) Repair Transformers (Pattern)

**Design details**
- Add targeted transformers that repair common issues so validators can be
  strict: e.g., `StageRepairTransformer` to flatten/renumber stages to 0..n.
- Always follow with the matching validator to confirm the fix.

**Milestones**
- M0: StageRepairTransformer + StageValidator pairing.

### 3) Diagnostics + Reporting
**Design details**
- Structured error objects with opcode, step, status, and control path context.
- Per-instruction summary: step counts pre/post optimization, control usage,
  and status coverage report.
- Optional debug dump: pretty-print expanded/optimized sequences for inspection.

**Milestones**
- M0: Structured error data model + formatter.
- M1: Compiler summary report (logging-backed, info level).
- M2: Debug dump of optimized sequences.

## Logging + Observability (New Project Proposal)

Add a cross-cutting logging system that can be shared by the compiler, sim,
assembler, and test harness:

- Select a C++ logging backend: **`absl::log`** (preferred).
- Provide a small facade with log levels, structured fields, and consistent
  formatting.
- Use this for compiler summaries, validator diagnostics, and sim runtime
  traceability.

This should likely precede deeper compiler work so diagnostics have a stable
home.

## Detailed Plan

1) **Pipeline parity scaffolding**
   - Add pass interfaces for validators/optimizers missing in IRATA2.
   - Insert validation runs between transforms to match pirata ordering.
   - Add focused unit tests per new pass.

2) **Validator suite**
   - Implement BusValidator with explicit bus writer/reader maps.
   - Implement StatusValidator with exact coverage checks.
   - Implement StageValidator for stage numbering invariants.
   - Add IRATA2-specific read/write ordering and control conflict checks.

3) **Optimizer suite**
   - EmptyStepOptimizer, DuplicateStepOptimizer, then Optimizer.
   - Verify behavior with golden microcode fixtures and reduction counts.
   - Ensure each optimizer is followed by full validator suite.

4) **Diagnostics**
   - Rich error payloads for validator/encoder failures.
   - Per-instruction compile summary with before/after counts.
   - Optional debug dump of expanded/optimized steps.

5) **Integration + CI**
   - Extend existing compiler tests for each stage and failure mode.
   - Add regression tests for microcode table stability.
   - Document opt-in flags if we gate optimizers initially.

## Open Questions

- Should the logging facade default to info-level summaries and debug-level
  details, or should a runtime flag gate debug logging globally?
