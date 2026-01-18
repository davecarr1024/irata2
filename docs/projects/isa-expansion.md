# ISA Expansion - Remaining Work

This document describes the remaining phases to complete a 6502-style instruction set.

## Current State

**Implemented (Phases 1-7):**
- ~60 instructions across 5 addressing modes
- Registers: A, X, PC, IR, SC, IPC, MAR, TMP
- Addressing modes: IMP, IMM, ZP, ABS, REL
- ALU: ADD, SUB, INC, DEC, AND, OR, XOR, ASL, LSR, ROL, ROR
- Status flags: Z, N, C, V

**Implemented Instructions by Category:**
| Category | Instructions |
|----------|--------------|
| System | HLT, NOP, CRS |
| Load/Store | LDA, STA, LDX, STX (IMM, ZP, ABS variants) |
| Arithmetic | ADC, SBC (IMM, ZP, ABS) |
| Logic | AND, ORA, EOR (IMM, ZP, ABS) |
| Shifts | ASL, LSR, ROL, ROR (IMP, ZP, ABS) |
| Compare | CMP (IMM, ZP, ABS) |
| Transfer | TAX, TXA |
| Inc/Dec | INX, DEX, INC, DEC (ZP, ABS) |
| Branches | BEQ, BNE, BCS, BCC, BMI, BPL, BVS, BVC (REL) |
| Jump | JEQ (ABS) |

## Missing from 6502

### Registers
- **Y register** - Second index register
- **SP (Stack Pointer)** - For stack operations

### Addressing Modes
- **ZPX** - Zero Page,X (ZP + X)
- **ZPY** - Zero Page,Y (ZP + Y, for LDX/STX only)
- **ABX** - Absolute,X (ABS + X)
- **ABY** - Absolute,Y (ABS + Y)
- **IND** - Indirect (JMP only)
- **IZX** - Indexed Indirect ((ZP,X))
- **IZY** - Indirect Indexed ((ZP),Y)

### Instructions
| Category | Missing Instructions |
|----------|---------------------|
| Y Register | LDY, STY, TAY, TYA, INY, DEY |
| Stack | PHA, PLA, PHP, PLP, TSX, TXS |
| Subroutine | JSR, RTS |
| Jump | JMP (ABS, IND) |
| Flags | CLC, SEC, CLV, (CLI, SEI, CLD, SED if needed) |
| Compare | CPX, CPY (IMM, ZP, ABS variants) |
| Bit Test | BIT (ZP, ABS) |
| Interrupt | BRK, RTI (optional) |

---

## Phase 8: Y Register & Indexed Addressing

**Goal:** Add Y register and indexed addressing modes (ZPX, ZPY, ABX, ABY).

### 8.0 Refactor: Fix Hidden State Anti-pattern

**Problem:** The current `ProgramCounter.add_offset` is a `ReadControl` that reads from the bus and stores the offset value internally. This violates the hardware-ish principle - state should be visible in the HDL component tree, not hidden in implementations.

**Current (bad):**
```cpp
class ProgramCounter : public Register<ProgramCounter, base::Word> {
  const ReadControl<base::Byte> add_offset_control_;  // Reads and stores internally
};
```

**Fixed (good):**
```cpp
class ProgramCounter : public Register<ProgramCounter, base::Word> {
  const ByteRegister signed_offset_;           // Explicit child register
  const ProcessControl<true> add_offset_control_;  // Uses signed_offset_.value()
};
```

**Microcode change:**
```yaml
# Before (single step, hidden read):
- [pc.add_offset]

# After (explicit register, then process):
- [memory.write, pc.signed_offset.read]
- [pc.add_signed_offset]
```

This refactor establishes the pattern for MAR's offset register. See `hdl/README.md` for the anti-pattern documentation.

### 8.1 Y Register Infrastructure

**HDL Changes:**
- Add ByteRegister `y` to CPU, same pattern as X register
- Controls: `y.write`, `y.read`, `y.reset`

**Sim Changes:**
- Add Y register to sim::Cpu following X register pattern
- Wire to data bus and status analyzer

### 8.2 Y Register Instructions (Implied)

| Instruction | Opcode | Description |
|-------------|--------|-------------|
| TAY | 0x33 | Transfer A to Y |
| TYA | 0x34 | Transfer Y to A |
| INY | 0x66 | Increment Y |
| DEY | 0x67 | Decrement Y |

**Microcode Pattern (TAY_IMP):**
```yaml
TAY_IMP:
  stages:
    - steps:
        - [a.write, y.read, status.analyzer.read]
```

### 8.3 LDY/STY Instructions

| Instruction | Mode | Opcode | Description |
|-------------|------|--------|-------------|
| LDY | IMM | 0x35 | Load Y (immediate) |
| LDY | ZP | 0x45 | Load Y (zero page) |
| LDY | ABS | 0x55 | Load Y (absolute) |
| STY | ZP | 0x46 | Store Y (zero page) |
| STY | ABS | 0x56 | Store Y (absolute) |

### 8.4 Zero Page Indexed Modes

**New Addressing Modes:**
```yaml
- name: ZeroPageX
  code: ZPX
  operands: 1
  description: "Zero page + X"

- name: ZeroPageY
  code: ZPY
  operands: 1
  description: "Zero page + Y"
```

**ZPX Instructions:** LDA, STA, ADC, SBC, AND, ORA, EOR, CMP, ASL, LSR, ROL, ROR, INC, DEC, LDY, STY

**ZPY Instructions:** LDX, STX (only these use Y indexing in 6502)

**Microcode Pattern (LDA_ZPX):**
```yaml
LDA_ZPX:
  stages:
    - steps:
        - [pc.write, memory.mar.read]
        - [memory.write, alu.lhs.read, pc.increment]
        - [x.write, alu.rhs.read]
        - [alu.opcode_bit_0]  # ADD
        - [alu.result.write, memory.mar.low.read]
        - [memory.mar.high.reset]
        - [memory.write, a.read, status.analyzer.read]
```

### 8.5 Absolute Indexed Modes

**New Addressing Modes:**
```yaml
- name: AbsoluteX
  code: ABX
  operands: 2
  description: "Absolute + X"

- name: AbsoluteY
  code: ABY
  operands: 2
  description: "Absolute + Y"
```

**ABX Instructions:** LDA, STA, ADC, SBC, AND, ORA, EOR, CMP, ASL, LSR, ROL, ROR, INC, DEC, LDY

**ABY Instructions:** LDA, STA, ADC, SBC, AND, ORA, EOR, CMP, LDX

**Microcode Pattern (LDA_ABX):**
```yaml
LDA_ABX:
  stages:
    - steps:
        # Fetch low byte of address
        - [pc.write, memory.mar.read]
        - [memory.write, tmp.low.read, pc.increment]
        # Fetch high byte of address
        - [pc.write, memory.mar.read]
        - [memory.write, tmp.high.read, pc.increment]
        # Add X to low byte
        - [tmp.low.write, alu.lhs.read]
        - [x.write, alu.rhs.read]
        - [alu.opcode_bit_0]  # ADD
        - [alu.result.write, memory.mar.low.read]
        # Add carry to high byte if needed (may need TMP increment)
        - [tmp.high.write, memory.mar.high.read]
        # Load value
        - [memory.write, a.read, status.analyzer.read]
```

**Note:** Page crossing (adding index causes carry into high byte) requires extra logic. Options:
1. Always add 1 cycle (simple, slightly slower)
2. Conditional on carry (authentic 6502 behavior, complex microcode)

**Recommendation:** Start with option 1, optimize later if needed.

### Phase 8 Deliverables
- Y register (HDL + sim)
- 6 Y register instructions (TAY, TYA, INY, DEY, LDY, STY)
- 4 new addressing modes (ZPX, ZPY, ABX, ABY)
- ~50 new instruction variants

---

## Phase 9: Stack Operations

**Goal:** Add stack pointer and push/pull/subroutine instructions.

### 9.1 Stack Pointer Register

**HDL Changes:**
- Add Counter<Byte> `sp` to CPU (needs increment/decrement)
- Controls: `sp.write`, `sp.read`, `sp.increment`, `sp.decrement`
- Initialize to 0xFF (stack at 0x01FF, grows downward)

**Stack Memory:** Page 0x01 (addresses 0x0100-0x01FF)

### 9.2 Push/Pull Instructions

| Instruction | Opcode | Description |
|-------------|--------|-------------|
| PHA | 0x80 | Push A to stack |
| PLA | 0x81 | Pull A from stack |
| PHP | 0x82 | Push status to stack |
| PLP | 0x83 | Pull status from stack |
| TSX | 0x84 | Transfer SP to X |
| TXS | 0x85 | Transfer X to SP |

**Microcode Pattern (PHA_IMP):**
```yaml
PHA_IMP:
  stages:
    - steps:
        - [sp.write, memory.mar.low.read]
        - [memory.mar.high.set_01]  # Stack page
        - [a.write, memory.read]
        - [sp.decrement]
```

**Microcode Pattern (PLA_IMP):**
```yaml
PLA_IMP:
  stages:
    - steps:
        - [sp.increment]
        - [sp.write, memory.mar.low.read]
        - [memory.mar.high.set_01]
        - [memory.write, a.read, status.analyzer.read]
```

### 9.3 Subroutine Instructions

| Instruction | Opcode | Description |
|-------------|--------|-------------|
| JSR | 0x86 | Jump to subroutine (absolute) |
| RTS | 0x87 | Return from subroutine |

**Microcode Pattern (JSR_ABS):**
```yaml
JSR_ABS:
  stages:
    - steps:
        # Read target address to TMP
        - [pc.write, memory.mar.read]
        - [memory.write, tmp.low.read, pc.increment]
        - [pc.write, memory.mar.read]
        - [memory.write, tmp.high.read, pc.increment]
        # Push PC high byte
        - [sp.write, memory.mar.low.read]
        - [memory.mar.high.set_01]
        - [pc.high.write, memory.read]
        - [sp.decrement]
        # Push PC low byte
        - [sp.write, memory.mar.low.read]
        - [memory.mar.high.set_01]
        - [pc.low.write, memory.read]
        - [sp.decrement]
        # Jump to subroutine
        - [tmp.write, pc.read]
```

**Microcode Pattern (RTS_IMP):**
```yaml
RTS_IMP:
  stages:
    - steps:
        # Pull PC low byte
        - [sp.increment]
        - [sp.write, memory.mar.low.read]
        - [memory.mar.high.set_01]
        - [memory.write, pc.low.read]
        # Pull PC high byte
        - [sp.increment]
        - [sp.write, memory.mar.low.read]
        - [memory.mar.high.set_01]
        - [memory.write, pc.high.read]
```

### Phase 9 Deliverables
- SP register (Counter with inc/dec)
- 8 instructions (PHA, PLA, PHP, PLP, TSX, TXS, JSR, RTS)
- Stack memory at page 0x01

---

## Phase 10: Status Flag Manipulation

**Goal:** Direct control over individual status flags.

### 10.1 Flag Set/Clear Controls

**HDL Changes:**
- Add `status.carry.set`, `status.carry.reset` controls
- Add `status.overflow.reset` control (CLV - only clear, no SEV in 6502)

| Instruction | Opcode | Description |
|-------------|--------|-------------|
| CLC | 0x90 | Clear carry flag |
| SEC | 0x91 | Set carry flag |
| CLV | 0x92 | Clear overflow flag |

**Optional (if interrupts/decimal mode added):**
- CLI/SEI - Clear/set interrupt disable
- CLD/SED - Clear/set decimal mode

**Microcode Pattern (SEC_IMP):**
```yaml
SEC_IMP:
  stages:
    - steps:
        - [status.carry.set]
```

### Phase 10 Deliverables
- 3 instructions (CLC, SEC, CLV)
- Status flag set/reset controls

---

## Phase 11: Compare X/Y and BIT Test

**Goal:** Complete comparison family and add BIT instruction.

### 11.1 CPX/CPY Instructions

| Instruction | Mode | Opcode | Description |
|-------------|------|--------|-------------|
| CPX | IMM | 0xA0 | Compare X (immediate) |
| CPX | ZP | 0xA1 | Compare X (zero page) |
| CPX | ABS | 0xA2 | Compare X (absolute) |
| CPY | IMM | 0xA3 | Compare Y (immediate) |
| CPY | ZP | 0xA4 | Compare Y (zero page) |
| CPY | ABS | 0xA5 | Compare Y (absolute) |

**Microcode Pattern:** Same as CMP but use X or Y instead of A.

### 11.2 BIT Instruction

| Instruction | Mode | Opcode | Description |
|-------------|------|--------|-------------|
| BIT | ZP | 0xA6 | Bit test (zero page) |
| BIT | ABS | 0xA7 | Bit test (absolute) |

**Behavior:**
- Z = (A & operand) == 0
- N = operand bit 7
- V = operand bit 6

**HDL Changes:**
- StatusAnalyzer may need enhancement to set N/V from memory operand bits rather than result

### Phase 11 Deliverables
- 8 instructions (CPX, CPY, BIT variants)

---

## Phase 12: Jump Instructions

**Goal:** Add unconditional jump and indirect jump.

### 12.1 JMP Absolute

| Instruction | Mode | Opcode | Description |
|-------------|------|--------|-------------|
| JMP | ABS | 0xB0 | Jump (absolute) |

**Microcode Pattern:**
```yaml
JMP_ABS:
  stages:
    - steps:
        - [pc.write, memory.mar.read]
        - [memory.write, tmp.low.read, pc.increment]
        - [pc.write, memory.mar.read]
        - [memory.write, tmp.high.read]
        - [tmp.write, pc.read]
```

### 12.2 JMP Indirect

| Instruction | Mode | Opcode | Description |
|-------------|------|--------|-------------|
| JMP | IND | 0xB1 | Jump (indirect) |

**New Addressing Mode:**
```yaml
- name: Indirect
  code: IND
  operands: 2
  description: "Address of address"
```

**Microcode Pattern:**
```yaml
JMP_IND:
  stages:
    - steps:
        # Read pointer address
        - [pc.write, memory.mar.read]
        - [memory.write, tmp.low.read, pc.increment]
        - [pc.write, memory.mar.read]
        - [memory.write, tmp.high.read]
        # Read target address from pointer
        - [tmp.write, memory.mar.read]
        - [memory.write, pc.low.read]
        - [tmp.increment]
        - [tmp.write, memory.mar.read]
        - [memory.write, pc.high.read]
```

**Note:** Requires TMP increment capability.

### Phase 12 Deliverables
- 2 instructions (JMP ABS, JMP IND)
- 1 new addressing mode (IND)

---

## Phase 13: Indirect Indexed Addressing (Optional)

**Goal:** Add complex pointer-based addressing modes.

### 13.1 Indexed Indirect (IZX)

**Addressing Mode:** `(ZP,X)` - Add X to zero page address, then indirect

**New Addressing Mode:**
```yaml
- name: IndexedIndirect
  code: IZX
  operands: 1
  description: "(ZP + X) points to address"
```

**Use Case:** Table of pointers in zero page.

**Instructions:** LDA, STA, ADC, SBC, AND, ORA, EOR, CMP

### 13.2 Indirect Indexed (IZY)

**Addressing Mode:** `(ZP),Y` - Indirect through zero page, then add Y

**New Addressing Mode:**
```yaml
- name: IndirectIndexed
  code: IZY
  operands: 1
  description: "(ZP) points to base, + Y"
```

**Use Case:** Pointer to array, indexed by Y.

**Instructions:** LDA, STA, ADC, SBC, AND, ORA, EOR, CMP

### Phase 13 Deliverables
- 2 new addressing modes (IZX, IZY)
- ~16 instruction variants

---

## Phase 14: Interrupts (Optional)

**Goal:** Add basic interrupt support.

### 14.1 Interrupt Instructions

| Instruction | Opcode | Description |
|-------------|--------|-------------|
| BRK | 0xC0 | Software interrupt |
| RTI | 0xC1 | Return from interrupt |

**HDL Changes:**
- Interrupt request line
- Interrupt disable flag (I) in status register
- Interrupt vector at 0xFFFE-0xFFFF

**Microcode:**
- BRK pushes PC and status, loads interrupt vector
- RTI pulls status and PC

### Phase 14 Deliverables
- 2 instructions (BRK, RTI)
- Interrupt mechanism

---

## Summary

### Instruction Count Projection

| Phase | New Instructions | Cumulative |
|-------|-----------------|------------|
| Current | 60 | 60 |
| Phase 8 (Y, Indexed) | ~56 | ~116 |
| Phase 9 (Stack) | 8 | ~124 |
| Phase 10 (Flags) | 3 | ~127 |
| Phase 11 (CPX/CPY/BIT) | 8 | ~135 |
| Phase 12 (JMP) | 2 | ~137 |
| Phase 13 (Indirect) | 16 | ~153 |
| Phase 14 (Interrupt) | 2 | ~155 |

### Addressing Mode Count

| Phase | Modes Added | Cumulative |
|-------|------------|------------|
| Current | IMP, IMM, ZP, ABS, REL | 5 |
| Phase 8 | ZPX, ZPY, ABX, ABY | 9 |
| Phase 12 | IND | 10 |
| Phase 13 | IZX, IZY | 12 |

### Recommended Order

1. **Phase 8** - Y register and indexed addressing (foundation for many programs)
2. **Phase 9** - Stack operations (enables subroutines)
3. **Phase 10** - Flag manipulation (simple, enables better control)
4. **Phase 11** - CPX/CPY/BIT (completes comparison family)
5. **Phase 12** - JMP (completes control flow)
6. **Phase 13** - Indirect addressing (optional, complex)
7. **Phase 14** - Interrupts (optional, system-level)

### Risk Assessment

| Phase | Risk | Rationale |
|-------|------|-----------|
| 8 | Medium | Y register straightforward; indexed addressing needs address calculation |
| 9 | High | SP register, complex multi-step microcode for JSR/RTS |
| 10 | Low | Simple flag controls |
| 11 | Medium | BIT has special flag behavior |
| 12 | Low-Medium | JMP IND needs TMP increment |
| 13 | High | Double indirection, complex microcode |
| 14 | High | New interrupt mechanism |
