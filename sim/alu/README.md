# ALU Design (MVP for LDA/CMP/JEQ)

This is the proposed ALU design for IRATA2, modeled after the pirata ALU.

## Goals

- Provide a stable, encoded ALU opcode interface (bit controls).
- Keep ALU operations decoupled from ISA instructions.
- Support CMP via SUB without writing back to A.
- Feed and update status flags via the status register using named Status views.

## Component Shape

ALU is a component with nested operation modules. Each module implements one
operation (e.g., NOP, ADD, SUB). ALU executes every Process phase based on
the current opcode bits; opcode 0 is NOOP.

### Inputs / Outputs

- `lhs` (ByteRegister, bus-connected)
- `rhs` (ByteRegister, bus-connected)
- `result` (ByteRegister, bus-connected)
- `carry_in` (status line, from Status Register via Status view)
- `carry_out` (status line, to Status Register via Status view)
- `overflow` (status line, to Status Register via Status view; optional for MVP)

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

This project uses a **status analyzer register** to set Z/N, while the ALU
updates C/V only:

- **ALU**: Sets C (and later V) via Status views.
- **Analyzer**: Updates Z/N every Process phase based on its register value.

For CMP, the ALU computes the subtraction result, writes it to `result`,
and microcode transfers that value to the analyzer to set Z/N.

## HDL + Sim Integration

### HDL

- Add `alu` component to CPU.
- `alu.lhs`, `alu.rhs`, `alu.result` are ByteRegisters on `data_bus`.
- `alu.opcode_bit_0..3` are ProcessControls.
- Status lines are `status.carry`, `status.overflow`, `status.zero`, `status.negative`.
- Status analyzer register is `status.analyzer` (bus-connected).

### Sim

- ALU component mirrors HDL.
- ALU uses Status views to read/set carry/overflow (no direct SR bit twiddling).
- Status analyzer reads from the data bus and updates Z/N every Process phase.
- The simulator initializes carry=1 so CMP uses no-borrow subtraction until
  SEC/CLC-style controls are added.

## Microcode Sketch (for LDA/CMP)

- **LDA #imm**
  - fetch preamble
  - `pc.write, memory.mar.low.read`
  - `memory.write, a.read, status.analyzer.read`
  - `pc.increment`

- **CMP #imm**
  - fetch preamble
  - `pc.write, memory.mar.low.read`
  - `memory.write, alu.rhs.read`
  - `a.write, alu.lhs.read`
  - `alu.opcode_bit_1` (SUB opcode)
  - `alu.result.write, status.analyzer.read`
  - `pc.increment`

## Open Questions

1) Should overflow be implemented for SUB now, or deferred?
2) Should carry_in be latched by a control (SEC/CLC) or read directly from status each tick?
3) Preferred ALU opcode map: stick to pirata’s map or customize for IRATA2?
