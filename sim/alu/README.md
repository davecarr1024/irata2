# ALU Design (MVP for LDA/CMP/JEQ)

This is the proposed ALU design for IRATA2, modeled after the pirata ALU.

## Goals

- Provide a stable, encoded ALU opcode interface (bit controls).
- Keep ALU operations decoupled from ISA instructions.
- Support CMP via SUB without writing back to A.
- Feed and update status flags via the status register.

## Component Shape

ALU is a component with nested operation modules. Each module implements one
operation (e.g., NOP, ADD, SUB). ALU executes every Process phase based on
the current opcode bits; opcode 0 is NOOP.

### Inputs / Outputs

- `lhs` (ByteRegister, bus-connected)
- `rhs` (ByteRegister, bus-connected)
- `result` (ByteRegister, bus-connected)
- `carry_in` (status line, from Status Register)
- `carry_out` (status line, to Status Register)
- `overflow` (status line, to Status Register; optional for MVP)
- `zero` / `negative` status lines (see status update policy below)

### Controls

- `opcode_bit_0..3` (ProcessControl): 4-bit opcode.
- opcode `0b0000` means NOOP.

LHS/RHS/RESULT are ordinary bus-connected registers and already provide
`read`/`write` controls via the Register base type.

## Opcode Map (pirata reference)

```
0000: NOP
0001: ADD  (lhs + rhs + carry_in)
0010: SUB  (lhs - rhs - borrow; borrow = !carry_in)
0011: AND
0100: OR
0101: XOR
0110: ASL
0111: LSR
1000: ROL
1001: ROR
```

The MVP only needs `NOP` and `SUB` to support CMP; LDA can bypass the ALU
entirely. Keep the full map as design intent for later extensions.

## Tick Behavior

- ALU decodes opcode bits during `Process` phase.
- Decoded module executes once per tick.
- Module writes `result` and updates status lines.
- For CMP: microcode reads `result` only for flags (no writeback to A).

## Status Updates

Two viable strategies:

1) **ALU updates C/V only**, Z/N updated by a separate status analyzer module
   when results are written to the bus (pirata-style).
2) **ALU updates C/V/Z/N directly** based on `result` for arithmetic ops.

For CMP, ALU should set Z/N (based on the subtraction result) and C (no borrow).

## HDL + Sim Integration

### HDL

- Add `alu` component to CPU.
- `alu.lhs`, `alu.rhs`, `alu.result` are ByteRegisters on `data_bus`.
- `alu.opcode_bit_0..3` are ProcessControls.
- Status lines are `status.carry`, `status.overflow`, `status.zero`, `status.negative`.

### Sim

- ALU component mirrors HDL.
- Modules implement operations and update status lines.
- `carry_in` reads from `status.carry` each tick (no separate control line).

## Microcode Sketch (for LDA/CMP)

- **LDA #imm**
  - fetch preamble
  - `pc.write, memory.mar.low.read`
  - `memory.write, a.read`
  - `pc.increment`

- **CMP #imm**
  - fetch preamble
  - `pc.write, memory.mar.low.read`
  - `memory.write, alu.rhs.read`
  - `a.write, alu.lhs.read`
  - `alu.opcode_bit_1` (SUB opcode)
  - (optional) flags update, no writeback
  - `pc.increment`

## Open Questions

1) Do you want Z/N updated by the ALU directly, or via a separate status analyzer?
2) Should we keep 4 opcode bits (16 ops) like pirata, or a different width?
3) For CMP, should ALU write result to a temp register or skip `result` entirely?
4) Should carry_in be latched by a control (SEC/CLC) or read directly from status each tick?
5) Should decimal mode be considered now or later (pirata has a D flag hook)?
6) Preferred ALU opcode map: stick to pirata’s map or customize for IRATA2?
