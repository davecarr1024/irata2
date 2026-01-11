# Microcode Compiler Improvements

## Goals

- Improve correctness with stronger validators.
- Reduce compiled microcode size and redundancy.
- Surface clearer errors and diagnostics for microcode authors.

## Scope

- New validators (bus usage, conflicting writes, dead control asserts).
- Optimizers (dead step removal, step merging, canonicalization).
- Compiler diagnostics (structured error output, source mapping hooks).

## Proposed Work

### 1) Validation Expansion
**Ideas**
- Bus usage validator: ensure a bus has a single writer per tick.
- Read-after-write validator: enforce ordering constraints on bus usage.
- Control conflict validator: detect mutually exclusive controls.
- Status/control coverage validator: ensure status-dependent variants are complete.

**Milestones**
- M0: Bus write conflict validator.
- M1: Read/write ordering validator.
- M2: Control conflict validator.

### 2) Optimization Passes
**Ideas**
- Dead step removal: prune steps that do not assert any controls.
- Step merging: fold identical consecutive steps.
- Control word canonicalization: reuse identical control patterns.

**Milestones**
- M0: Dead step removal with tests.
- M1: Step merging with tests.
- M2: Optional control word canonicalization.

### 3) Diagnostics + Reporting
**Ideas**
- Structured error output (machine-readable + human-friendly).
- Emit per-instruction compile summary (step counts, control usage).
- Optional debug dump of optimized sequences.

**Milestones**
- M0: Structured error data model.
- M1: Compiler summary report.
- M2: Debug dump of optimized sequences.

## Open Questions

- Should optimizer passes be opt-in or enabled by default?
- How should compile summaries be surfaced (stdout vs file)?
