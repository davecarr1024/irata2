# ISA Expansion (Complete)

The 6502-style ISA expansion is finished.

## Current State

- 152 instructions across 12 addressing modes
- Registers: A, X, Y, SP, PC, IR, SC, IPC, MAR, TMP
- ALU ops include add/sub, logic, shifts/rotates, inc/dec
- Status flags: Z, N, C, V, I

## Notes

No remaining work is tracked here. Historical implementation details live in git history.
If new ISA work is added later, track it in `docs/plan.md` and update:

- `isa/instructions.yaml`
- `microcode/microcode.yaml`
- `assembler/test/asm_tests/`
