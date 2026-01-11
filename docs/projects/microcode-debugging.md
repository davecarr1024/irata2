# Microcode Debugging and Introspection

## Goals

- Make compiled microcode easy to inspect and reason about.
- Provide a reversible view from compiled ROM back to readable steps.

## Problem Statement

The current microcode pipeline compiles YAML into IR and then into a ROM
format. Debugging the final control words is difficult without tooling to
render the compiled output back into a readable form.

## Proposed Work

1. **Microcode decompiler**
   - Emit a YAML-like view of compiled microcode.
   - Show opcode, status variants, stages, and steps.

2. **Control word decoding**
   - Provide named control bits and their asserted state.
   - Map bits to HDL control paths for clarity.

3. **Diff tooling**
   - Compare source YAML vs compiled output.
   - Highlight auto-inserted controls (increment/reset).

4. **CLI integration**
   - Add a command (or flag) to dump the compiled microcode.

## Milestones

- M0: Decoder that dumps control words to readable lists.
- M1: Full YAML-like dump per opcode/stage/step.
- M2: CLI/CI utility for automated inspection.

## Open Questions

- Should output be deterministic and sorted by control path?
- Where should the CLI entry live (microcode tool vs sim tool)?
