# ISA Expansion (Batched)

## Goals

- Grow the ISA in coherent batches aligned with hardware blocks.
- Keep microcode and sim behavior consistent with HDL structure.
- Maintain incremental, test-driven progress.

## Proposed Batches

1. **ALU instructions**
   - ADD, SUB, AND, OR, XOR, INC, DEC, shifts/rotates.
   - Immediate + basic register/memory addressing modes.

2. **Register transfer**
   - A <-> X, A <-> memory, X <-> memory.
   - Move and load/store families.

3. **Stack**
   - Push/pop A, flags, PC for call/return.
   - Stack pointer register and stack page conventions.

4. **Subroutines and branches**
   - JSR/RTS, relative branches, compare/branch combos.

5. **Addressing modes**
   - Zero page, absolute, indexed, indirect, etc.

## Work Pattern

- Expand ISA definitions + assembler encoding together.
- Add microcode entries and tests before wiring new HDL/sim components.
- Add ASM integration tests per new instruction group.

## Milestones

- M0: Define and implement ALU instruction batch with tests.
- M1: Add stack and subroutine primitives.
- M2: Expand addressing modes and branch coverage.

## Open Questions

- Which 6502 behaviors should be matched vs simplified?
- What is the priority order for addressing modes?
