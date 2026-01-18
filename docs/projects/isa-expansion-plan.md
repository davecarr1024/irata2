# ISA Expansion - Detailed Implementation Plan

## Overview

This document provides a phased, detailed plan for expanding the IRATA2 instruction set. The plan groups instructions by functionality and addressing modes by complexity, building incrementally on the existing infrastructure.

## Current State (Baseline)

**Implemented Instructions (~60 total):**
- System: HLT, NOP, CRS
- Loads/Stores: LDA/STA/LDX/STX (IMM, ZP, ABS)
- Compare: CMP (IMM, ZP, ABS)
- ALU: ADC/SBC/AND/ORA/EOR (IMM, ZP, ABS)
- Shifts: ASL/LSR/ROL/ROR (IMP, ZP, ABS)
- Transfers: TAX, TXA
- Inc/Dec: INX/DEX + INC/DEC (ZP, ABS)
- Branch: JEQ (ABS), BEQ/BNE/BCS/BCC/BMI/BPL/BVS/BVC (REL)

**Implemented Addressing Modes (5 total):**
- Implied (IMP) - 0 operands
- Immediate (IMM) - 1 operand
- Zero Page (ZP) - 1 operand
- Absolute (ABS) - 2 operands
- Relative (REL) - 1 operand

**Current ALU Support:**
- 4-bit opcode (16 possible operations)
- Implemented: SUB (opcode 0x2)
- Available: 15 more opcodes

**Current Registers:**
- A (accumulator)
- X (index register)
- PC (program counter)
- IR (instruction register)
- SC (step counter)
- IPC (instruction PC)
- MAR (memory address register)
- TMP (temporary word register - NEW!)

**Status Flags:**
- Z (zero)
- N (negative)
- C (carry)
- V (overflow)

## Design Principles

1. **Incremental Growth** - Add instructions in small, testable batches
2. **Hardware-First** - Implement ALU operations before instructions that use them
3. **Address Mode Progression** - Start simple (immediate), add complexity gradually
4. **Test Coverage** - Each batch includes ASM integration tests
5. **Microcode Validation** - Use validators/optimizers to ensure correctness
6. **6502-Inspired** - Follow 6502 conventions where reasonable, simplify where beneficial

## Phase 1: ALU Core Operations ✓ COMPLETE

**Goal:** Implement fundamental ALU operations used by multiple instruction families.

**Status:** Complete - All 11 ALU operations implemented with 61 comprehensive unit tests. All 359 tests passing.

**Implementation Summary:**
- Phase 1.1: ADD (0x1), INC (0x3) - 17 tests
- Phase 1.2: AND (0x4), OR (0x5), XOR (0x6) - 18 tests
- Phase 1.3: ASL (0x7), LSR (0x8), ROL (0x9), ROR (0xA) - 20 tests
- Phase 1.4: DEC (0xB) - 6 tests
- Existing: SUB (0x2)

**Files Modified:**
- `sim/src/alu/alu.cpp` - Added 10 new ALU operation cases
- `sim/test/alu_test.cpp` - Created with 61 comprehensive tests
- `sim/test/CMakeLists.txt` - Added alu_test.cpp to build

### 1.1 Basic Arithmetic (Addition) ✓

**ALU Opcodes:**
- `0x1` - ADD (with carry in)
- `0x3` - INC (add 1, ignore carry in)

**Changes:**
- Update `sim/src/alu/alu.cpp` TickProcess() with ADD/INC cases
- Add unit tests in `sim/test/alu_test.cpp`

**Test Coverage:**
- ADD: various operands with/without carry
- INC: boundary cases (0xFF wraps to 0x00)
- Flag behavior: Z, N, C, V

**Estimated Complexity:** Low (extend existing ALU pattern)

**Status:** ✓ Complete

### 1.2 Basic Logic Operations ✓

**ALU Opcodes:**
- `0x4` - AND (bitwise and)
- `0x5` - OR (bitwise or)
- `0x6` - XOR (bitwise exclusive or)

**Changes:**
- Update `sim/src/alu/alu.cpp` TickProcess() with AND/OR/XOR cases
- Add unit tests

**Test Coverage:**
- Each operation with various bit patterns
- Flag behavior: Z, N (C and V cleared)

**Estimated Complexity:** Low (no carry/overflow logic)

**Status:** ✓ Complete

### 1.3 Shifts and Rotates ✓

**ALU Opcodes:**
- `0x7` - ASL (arithmetic shift left)
- `0x8` - LSR (logical shift right)
- `0x9` - ROL (rotate left through carry)
- `0xA` - ROR (rotate right through carry)

**Changes:**
- Update `sim/src/alu/alu.cpp` TickProcess() with shift/rotate cases
- Add unit tests

**Test Coverage:**
- Each operation with various bit patterns
- Carry flag interactions
- Flag behavior: Z, N, C

**Estimated Complexity:** Medium (carry flag interactions)

**Status:** ✓ Complete

### 1.4 Remaining Arithmetic ✓

**ALU Opcodes:**
- `0xB` - DEC (subtract 1, ignore carry)
- (SUB already implemented as 0x2)

**Changes:**
- Update `sim/src/alu/alu.cpp` TickProcess() with DEC case
- Add unit tests

**Test Coverage:**
- DEC: boundary cases (0x00 wraps to 0xFF)
- Flag behavior: Z, N

**Estimated Complexity:** Low (mirror of INC)

**Status:** ✓ Complete

**Phase 1 Deliverable:** ✓ ALU supports 11 operations total (SUB + 10 new)

## Phase 2: Immediate Mode Instructions ✓ COMPLETE

**Goal:** Add instructions using existing immediate addressing mode with new ALU operations.

**Rationale:** Immediate mode is already implemented for LDA/CMP. This phase adds functionality without addressing mode complexity.

**Status:** Complete - All 9 instructions implemented with 56 integration tests and 9 e2e assembly tests. All 440 tests passing.

### 2.1 Arithmetic Instructions (Immediate)

**New Instructions:**
- `ADC` - Add with Carry (immediate)
- `SBC` - Subtract with Carry (immediate)

**Changes:**
- Add to `isa/instructions.yaml`
- Add microcode in `microcode/microcode.yaml`
- Add assembler test in `assembler/test/asm_tests/`

**Microcode Pattern (ADC_IMM):**
```yaml
ADC_IMM:
  stages:
    - steps:
        - [pc.write, memory.mar.read]
        - [memory.write, alu.rhs.read, pc.increment]
        - [a.write, alu.lhs.read]
        - [alu.opcode_bit_0]  # opcode 0x1 = ADD
        - [alu.result.write, a.read, status.analyzer.read]
```

**Estimated Complexity:** Low (follow LDA/CMP pattern)

### 2.2 Logic Instructions (Immediate)

**New Instructions:**
- `AND` - Bitwise AND (immediate)
- `ORA` - Bitwise OR (immediate)
- `EOR` - Bitwise XOR (immediate)

**Changes:**
- Add to `isa/instructions.yaml`
- Add microcode in `microcode/microcode.yaml`
- Add assembler tests

**Microcode Pattern:** Similar to ADC_IMM, different ALU opcode bits

**Estimated Complexity:** Low (same pattern, different opcodes)

### 2.3 Shift/Rotate Instructions (Accumulator)

**Note:** Shifts/rotates typically operate on accumulator or memory, not immediate values.

**New Instructions:**
- `ASL` - Arithmetic Shift Left (accumulator/implied)
- `LSR` - Logical Shift Right (accumulator/implied)
- `ROL` - Rotate Left (accumulator/implied)
- `ROR` - Rotate Right (accumulator/implied)

**Microcode Pattern (ASL_IMP):**
```yaml
ASL_IMP:
  stages:
    - steps:
        - [a.write, alu.lhs.read]
        - [alu.opcode_bit_0, alu.opcode_bit_1, alu.opcode_bit_2]  # opcode 0x7 = ASL
        - [alu.result.write, a.read, status.analyzer.read]
```

**Estimated Complexity:** Low (implied mode already exists)

**Status:** ✓ Complete

**Phase 2 Deliverable:** ✓ 9 new instructions implemented:
- Arithmetic: ADC_IMM (0x20), SBC_IMM (0x21)
- Logic: AND_IMM (0x22), ORA_IMM (0x23), EOR_IMM (0x24)
- Shifts: ASL_IMP (0x25), LSR_IMP (0x26), ROL_IMP (0x27), ROR_IMP (0x28)

## Phase 3: Register Transfer Operations ✓ COMPLETE

**Goal:** Move data between registers and enable X register usage.

### 3.1 Register-to-Register Transfers (Implied)

**New Instructions:**
- `TAX` - Transfer A to X
- `TXA` - Transfer X to A

**Microcode Pattern (TAX_IMP):**
```yaml
TAX_IMP:
  stages:
    - steps:
        - [a.write, x.read, status.analyzer.read]
```

**Estimated Complexity:** Low (simple register copy)

### 3.2 Load X Register

**New Instructions:**
- `LDX` - Load X (immediate)

**Microcode Pattern:** Similar to LDA_IMM, write to X instead of A

**Estimated Complexity:** Low

**Phase 3 Deliverable:** ✓ 3 new instructions (register operations)

## Phase 4: Zero Page Addressing Mode ✓ COMPLETE

**Goal:** Add efficient single-byte address mode for common memory operations.

### 4.1 Define Zero Page Mode

**Add to `isa/instructions.yaml`:**
```yaml
- name: ZeroPage
  code: ZP
  operands: 1
  description: "Zero page address (0x00XX)"
```

**Rationale:** Zero page uses high byte = 0x00, so only one operand byte needed.

### 4.2 Zero Page Memory Access

**New Instructions:**
- `LDA` - Load A (zero page)
- `STA` - Store A (zero page)
- `LDX` - Load X (zero page)
- `STX` - Store X (zero page)

**Microcode Pattern (LDA_ZP):**
```yaml
LDA_ZP:
  stages:
    - steps:
        - [pc.write, memory.mar.read]
        - [memory.write, memory.mar.low.read, pc.increment]  # low byte from operand
        - [memory.mar.high.reset]  # high byte = 0x00
        - [memory.write, a.read, status.analyzer.read]
```

**Microcode Pattern (STA_ZP):**
```yaml
STA_ZP:
  stages:
    - steps:
        - [pc.write, memory.mar.read]
        - [memory.write, memory.mar.low.read, pc.increment]
        - [memory.mar.high.reset]
        - [a.write, memory.read]
```

**Estimated Complexity:** Medium (new addressing mode pattern)

### 4.3 Zero Page ALU Operations

**New Instructions:**
- `ADC` - Add with Carry (zero page)
- `SBC` - Subtract with Carry (zero page)
- `AND` - Bitwise AND (zero page)
- `ORA` - Bitwise OR (zero page)
- `EOR` - Bitwise XOR (zero page)
- `CMP` - Compare A (zero page)

**Microcode Pattern (ADC_ZP):**
```yaml
ADC_ZP:
  stages:
    - steps:
        - [pc.write, memory.mar.read]
        - [memory.write, memory.mar.low.read, pc.increment]
        - [memory.mar.high.reset]
        - [memory.write, alu.rhs.read]
        - [a.write, alu.lhs.read]
        - [alu.opcode_bit_0]  # opcode for ADD
        - [alu.result.write, a.read, status.analyzer.read]
```

**Estimated Complexity:** Medium (combine ZP addressing + ALU ops)

### 4.4 Zero Page Shifts

**New Instructions:**
- `ASL` - Arithmetic Shift Left (zero page)
- `LSR` - Logical Shift Right (zero page)
- `ROL` - Rotate Left (zero page)
- `ROR` - Rotate Right (zero page)

**Microcode Pattern (ASL_ZP):**
```yaml
ASL_ZP:
  stages:
    - steps:
        - [pc.write, memory.mar.read]
        - [memory.write, memory.mar.low.read, pc.increment]
        - [memory.mar.high.reset]
        - [memory.write, alu.lhs.read]
        - [alu.opcode_bit_0, alu.opcode_bit_1, alu.opcode_bit_2]  # ASL opcode
        - [alu.result.write, memory.read, status.analyzer.read]
```

**Note:** Read-modify-write pattern - read from memory, ALU operates, write back to memory.

**Estimated Complexity:** Medium (read-modify-write pattern)

**Phase 4 Deliverable:** ✓ 1 new addressing mode + 14 new instruction variants

## Phase 5: Absolute Addressing Mode Expansion ✓ COMPLETE

**Goal:** Expand absolute mode beyond JEQ to include loads, stores, and ALU operations.

**Note:** Absolute mode (ABS) already exists for JEQ. This phase adds data operations.

### 5.1 Absolute Memory Access

**New Instructions:**
- `LDA` - Load A (absolute)
- `STA` - Store A (absolute)
- `LDX` - Load X (absolute)
- `STX` - Store X (absolute)

**Microcode Pattern (LDA_ABS):**
```yaml
LDA_ABS:
  stages:
    - steps:
        - [pc.write, memory.mar.read]
        - [memory.write, memory.mar.low.read, pc.increment]
        - [pc.write, memory.mar.read]
        - [memory.write, memory.mar.high.read, pc.increment]
        - [memory.write, a.read, status.analyzer.read]
```

**Estimated Complexity:** Low (follow JEQ_ABS pattern for address loading)

### 5.2 Absolute ALU Operations

**New Instructions:**
- `ADC` - Add with Carry (absolute)
- `SBC` - Subtract with Carry (absolute)
- `AND` - Bitwise AND (absolute)
- `ORA` - Bitwise OR (absolute)
- `EOR` - Bitwise XOR (absolute)
- `CMP` - Compare A (absolute)

**Estimated Complexity:** Medium (absolute addressing + ALU)

### 5.3 Absolute Shifts

**New Instructions:**
- `ASL`, `LSR`, `ROL`, `ROR` (absolute)

**Estimated Complexity:** Medium (read-modify-write with 2-byte address)

**Phase 5 Deliverable:** ✓ 14 new instruction variants (absolute mode)

## Phase 6: Increment/Decrement Instructions ✓ COMPLETE

**Goal:** Add memory and register increment/decrement operations.

### 6.1 Register Inc/Dec (Implied)

**New Instructions:**
- `INX` - Increment X
- `DEX` - Decrement X

**Microcode Pattern (INX_IMP):**
```yaml
INX_IMP:
  stages:
    - steps:
        - [x.write, alu.lhs.read]
        - [alu.opcode_bit_0, alu.opcode_bit_1]  # INC opcode
        - [alu.result.write, x.read, status.analyzer.read]
```

**Estimated Complexity:** Low

### 6.2 Memory Inc/Dec

**New Instructions:**
- `INC` - Increment memory (zero page)
- `DEC` - Decrement memory (zero page)
- `INC` - Increment memory (absolute)
- `DEC` - Decrement memory (absolute)

**Microcode Pattern:** Read-modify-write like shifts

**Estimated Complexity:** Medium (read-modify-write pattern)

**Phase 6 Deliverable:** ✓ 6 new instructions

## Phase 7: Conditional Branches ✓ COMPLETE

**Goal:** Complete branch instruction family.

### 7.1 Status Flag Branches (Relative Addressing)

**New Addressing Mode:**
```yaml
- name: Relative
  code: REL
  operands: 1
  description: "Signed byte offset from PC"
```

**New Instructions:**
- `BEQ` - Branch if Equal (Z=1)
- `BNE` - Branch if Not Equal (Z=0)
- `BCS` - Branch if Carry Set (C=1)
- `BCC` - Branch if Carry Clear (C=0)
- `BMI` - Branch if Minus (N=1)
- `BPL` - Branch if Plus (N=0)
- `BVS` - Branch if Overflow Set (V=1)
- `BVC` - Branch if Overflow Clear (V=0)

**Microcode Pattern (BEQ_REL):**
```yaml
BEQ_REL:
  variants:
    - when: { zero: true }
      stages:
        - steps:
            - [pc.write, memory.mar.read]
            - [memory.write, pc.increment, pc.add_offset]
    - when: { zero: false }
      stages:
        - steps:
            - [pc.write, memory.mar.read]
            - [memory.write, pc.increment]
```

**Note:** Relative branches use the ProgramCounter `pc.add_offset` control, which latches a signed byte from the data bus and applies it during the Process phase.

**Estimated Complexity:** Medium (PC manipulation)

### 7.2 Replace JEQ with BEQ

**Deprecation:** Once BEQ_REL works, migrate tests from JEQ_ABS to BEQ_REL.

**Rationale:** Relative branches are more efficient and 6502-like than absolute jumps for conditionals.

**Phase 7 Deliverable:** ✓ 1 new addressing mode + 8 new branch instructions

## Phase 8: Indexed Addressing Modes

**Goal:** Enable array/table access using X register as index.

### 8.1 Zero Page Indexed (X)

**New Addressing Mode:**
```yaml
- name: ZeroPageX
  code: ZPX
  operands: 1
  description: "Zero page address + X"
```

**New Instructions:**
- `LDA` - Load A (zero page, X)
- `STA` - Store A (zero page, X)
- `ADC`, `SBC`, `AND`, `ORA`, `EOR`, `CMP` (zero page, X)
- `ASL`, `LSR`, `ROL`, `ROR`, `INC`, `DEC` (zero page, X)

**Microcode Pattern (LDA_ZPX):**
```yaml
LDA_ZPX:
  stages:
    - steps:
        - [pc.write, memory.mar.read]
        - [memory.write, alu.lhs.read, pc.increment]  # base address
        - [x.write, alu.rhs.read]  # X as offset
        - [alu.opcode_bit_0]  # ADD
        - [alu.result.write, memory.mar.low.read]
        - [memory.mar.high.reset]
        - [memory.write, a.read, status.analyzer.read]
```

**Estimated Complexity:** Medium (address calculation)

### 8.2 Absolute Indexed (X)

**New Addressing Mode:**
```yaml
- name: AbsoluteX
  code: ABX
  operands: 2
  description: "Absolute address + X"
```

**New Instructions:**
- Same families as ZPX but with 2-byte base address

**Microcode Pattern:** Similar to ZPX but load 2-byte address first (like ABS)

**Estimated Complexity:** Medium-High (16-bit address + index calculation)

### 8.3 Absolute Indexed (Y) - Optional

**New Register:** Y (index register)

**Rationale:** 6502 has both X and Y for dual indexing. Defer to later phase if needed.

**Phase 8 Deliverable:** 2 new addressing modes + ~24 new instruction variants

## Phase 9: Stack Operations

**Goal:** Enable subroutine calls and stack-based data management.

### 9.1 Stack Pointer Register

**New Register:** SP (stack pointer)

**HDL/Sim Changes:**
- Add ByteRegister for SP in CPU
- Connect to data bus and/or address bus
- Initialize to 0xFF (stack at 0x01FF, grows downward)

**Controls:**
- `sp.write`, `sp.read`, `sp.reset`
- `sp.increment`, `sp.decrement` (may need Counter instead of Register)

**Estimated Complexity:** Medium (new register, stack conventions)

### 9.2 Push/Pop Instructions (Implied)

**New Instructions:**
- `PHA` - Push Accumulator
- `PLA` - Pull Accumulator
- `PHP` - Push Processor Status
- `PLP` - Pull Processor Status

**Microcode Pattern (PHA_IMP):**
```yaml
PHA_IMP:
  stages:
    - steps:
        - [sp.write, memory.mar.low.read]
        - [memory.mar.high.set]  # high byte = 0x01 for stack page
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
        - [memory.mar.high.set]
        - [memory.write, a.read, status.analyzer.read]
```

**Estimated Complexity:** Medium (stack pointer manipulation)

### 9.3 Subroutine Instructions

**New Instructions:**
- `JSR` - Jump to Subroutine (absolute)
- `RTS` - Return from Subroutine (implied)

**Microcode Pattern (JSR_ABS):**
```yaml
JSR_ABS:
  stages:
    - steps:
        # Read subroutine address to TMP
        - [pc.write, memory.mar.read]
        - [memory.write, tmp.low.read, pc.increment]
        - [pc.write, memory.mar.read]
        - [memory.write, tmp.high.read, pc.increment]
        # Push return address (PC) to stack
        - [sp.write, memory.mar.low.read]
        - [memory.mar.high.set]
        - [pc.high.write, memory.read]
        - [sp.decrement]
        - [sp.write, memory.mar.low.read]
        - [memory.mar.high.set]
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
        # Pull return address from stack
        - [sp.increment]
        - [sp.write, memory.mar.low.read]
        - [memory.mar.high.set]
        - [memory.write, pc.low.read]
        - [sp.increment]
        - [sp.write, memory.mar.low.read]
        - [memory.mar.high.set]
        - [memory.write, pc.high.read]
```

**Estimated Complexity:** High (complex multi-step sequences, PC manipulation)

**Phase 9 Deliverable:** 1 new register + 6 new instructions

## Phase 10: Advanced Addressing Modes

**Goal:** Add indirect and indexed-indirect modes for pointer-based operations.

### 10.1 Indirect Addressing (JMP only)

**New Addressing Mode:**
```yaml
- name: Indirect
  code: IND
  operands: 2
  description: "Address of address (16-bit pointer)"
```

**New Instruction:**
- `JMP` - Jump (indirect)

**Microcode Pattern (JMP_IND):**
```yaml
JMP_IND:
  stages:
    - steps:
        # Read pointer address to MAR
        - [pc.write, memory.mar.read]
        - [memory.write, tmp.low.read, pc.increment]
        - [pc.write, memory.mar.read]
        - [memory.write, tmp.high.read, pc.increment]
        # Read target address from pointer
        - [tmp.write, memory.mar.read]
        - [memory.write, pc.low.read]
        - [tmp.increment]  # assumes TMP can increment
        - [tmp.write, memory.mar.read]
        - [memory.write, pc.high.read]
```

**Estimated Complexity:** High (double indirection)

### 10.2 Indexed Indirect (Zero Page, X)

**New Addressing Mode:**
```yaml
- name: IndexedIndirect
  code: IZX
  operands: 1
  description: "(ZP + X) points to address"
```

**Use Case:** Table of pointers in zero page, indexed by X.

**New Instructions:**
- `LDA`, `STA`, `ADC`, `SBC`, `AND`, `ORA`, `EOR`, `CMP` (indexed indirect)

**Estimated Complexity:** Very High (ZP + X calculation, then indirection)

### 10.3 Indirect Indexed (Zero Page), Y

**New Addressing Mode:**
```yaml
- name: IndirectIndexed
  code: IZY
  operands: 1
  description: "(ZP) points to base, + Y"
```

**Use Case:** Pointer in zero page to array, indexed by Y.

**New Instructions:**
- `LDA`, `STA`, `ADC`, `SBC`, `AND`, `ORA`, `EOR`, `CMP` (indirect indexed)

**Estimated Complexity:** Very High (indirection, then Y addition with carry)

**Phase 10 Deliverable:** 3 new addressing modes + ~17 new instruction variants

## Phase 11: Jump Instructions

**Goal:** Complete jump instruction family.

### 11.1 Unconditional Jump

**New Instructions:**
- `JMP` - Jump (absolute) - already have JEQ pattern, make unconditional

**Microcode Pattern (JMP_ABS):**
```yaml
JMP_ABS:
  stages:
    - steps:
        - [pc.write, memory.mar.read]
        - [memory.write, tmp.low.read, pc.increment]
        - [pc.write, memory.mar.read]
        - [memory.write, tmp.high.read, pc.increment]
        - [tmp.write, pc.read]
```

**Estimated Complexity:** Low (like JEQ without condition)

**Phase 11 Deliverable:** 1 instruction (+ IND variant from Phase 10)

## Phase 12: Status Flag Manipulation

**Goal:** Direct control over status flags.

### 12.1 Flag Set/Clear (Implied)

**New Instructions:**
- `SEC` - Set Carry
- `CLC` - Clear Carry
- `SEI` - Set Interrupt Disable (if interrupt support added)
- `CLI` - Clear Interrupt Disable
- `SED` - Set Decimal Mode (if decimal mode added)
- `CLD` - Clear Decimal Mode
- `CLV` - Clear Overflow

**Microcode Pattern (SEC_IMP):**
```yaml
SEC_IMP:
  stages:
    - steps:
        - [status.carry.set]
```

**Microcode Pattern (CLC_IMP):**
```yaml
CLC_IMP:
  stages:
    - steps:
        - [status.carry.reset]
```

**Note:** May need individual set/reset controls for each status flag.

**Estimated Complexity:** Low-Medium (depends on status register control design)

**Phase 12 Deliverable:** 7 new instructions

## Phase 13: Comparison Instructions

**Goal:** Complete comparison instruction family.

### 13.1 X and Y Comparisons

**New Instructions:**
- `CPX` - Compare X (immediate, zero page, absolute)
- `CPY` - Compare Y (immediate, zero page, absolute)

**Microcode Pattern:** Like CMP but use X/Y instead of A

**Estimated Complexity:** Low (follow CMP pattern)

**Phase 13 Deliverable:** 6 new instruction variants

## Phase 14: Bit Test Instruction

**Goal:** Add bit testing capability.

### 14.1 BIT Instruction

**New Instruction:**
- `BIT` - Bit Test (zero page, absolute)

**Behavior:**
- Z = (A & operand) == 0
- N = operand bit 7
- V = operand bit 6

**Microcode Pattern (BIT_ZP):**
```yaml
BIT_ZP:
  stages:
    - steps:
        - [pc.write, memory.mar.read]
        - [memory.write, memory.mar.low.read, pc.increment]
        - [memory.mar.high.reset]
        - [memory.write, alu.rhs.read]
        - [a.write, alu.lhs.read]
        - [alu.opcode_bit_2]  # AND opcode
        - [alu.result.write, status.analyzer.read]
        # TODO: Also set N/V from memory operand bits 7/6
```

**Note:** May need StatusAnalyzer enhancement to handle N/V from operand, not result.

**Estimated Complexity:** Medium (special flag behavior)

**Phase 14 Deliverable:** 2 new instruction variants

## Summary Tables

### Instruction Count by Phase

| Phase | New Instructions | New Variants | Cumulative Total |
|-------|-----------------|--------------|------------------|
| Baseline | 6 | 0 | 6 |
| 1 (ALU ops) | 0 | 0 | 6 |
| 2 (Immediate) | 9 | 0 | 15 |
| 3 (Register) | 3 | 0 | 18 |
| 4 (Zero Page) | 4 | 14 | 32 |
| 5 (Absolute) | 0 | 14 | 46 |
| 6 (Inc/Dec) | 6 | 0 | 52 |
| 7 (Branches) | 8 | 0 | 60 |
| 8 (Indexed) | 0 | 24 | 84 |
| 9 (Stack) | 6 | 0 | 90 |
| 10 (Indirect) | 1 | 17 | 107 |
| 11 (Jump) | 1 | 0 | 108 |
| 12 (Flags) | 7 | 0 | 115 |
| 13 (Compare) | 0 | 6 | 121 |
| 14 (BIT) | 0 | 2 | 123 |

**Target:** ~128 instructions (close to 6502's 56 official opcodes × variants)

### Addressing Mode Count by Phase

| Phase | Addressing Modes Added | Cumulative Total |
|-------|------------------------|------------------|
| Baseline | IMP, IMM, ABS | 3 |
| 4 | ZP | 4 |
| 7 | REL | 5 |
| 8 | ZPX, ABX | 7 |
| 10 | IND, IZX, IZY | 10 |

**Target:** 10 addressing modes (matches 6502 core modes)

### ALU Opcode Allocation

| Opcode | Operation | Phase | Status |
|--------|-----------|-------|--------|
| 0x0 | NOP | - | Reserved (no-op) |
| 0x1 | ADD | 1.1 | ✓ Implemented |
| 0x2 | SUB | - | ✓ Implemented |
| 0x3 | INC | 1.1 | ✓ Implemented |
| 0x4 | AND | 1.2 | ✓ Implemented |
| 0x5 | OR | 1.2 | ✓ Implemented |
| 0x6 | XOR | 1.2 | ✓ Implemented |
| 0x7 | ASL | 1.3 | ✓ Implemented |
| 0x8 | LSR | 1.3 | ✓ Implemented |
| 0x9 | ROL | 1.3 | ✓ Implemented |
| 0xA | ROR | 1.3 | ✓ Implemented |
| 0xB | DEC | 1.4 | ✓ Implemented |
| 0xC-0xF | - | - | Reserved (future) |

## Implementation Strategy

### Per-Phase Workflow

For each phase:

1. **Plan** (if applicable)
   - Update HDL if new components needed
   - Design microcode patterns
   - Identify test cases

2. **Implement**
   - Update `isa/instructions.yaml` (new instructions/modes)
   - Update `microcode/microcode.yaml` (microcode sequences)
   - Update `sim/src/alu/alu.cpp` (ALU operations if needed)
   - Update HDL/sim for new registers/controls if needed

3. **Test**
   - Add unit tests for new ALU operations
   - Add ASM integration tests for new instructions
   - Run full test suite (`make test`)

4. **Validate**
   - Check microcode validators pass
   - Review coverage report
   - Test edge cases (boundary values, flag interactions)

5. **Commit**
   - One commit per instruction batch or sub-phase
   - Descriptive commit messages
   - Push and verify CI passes

### Recommended Phase Order

**Defensive approach (stability first):**
1. Phase 1 (ALU) - Foundation for all arithmetic/logic instructions
2. Phase 2 (Immediate) - Low-risk, high-value functionality
3. Phase 3 (Register) - Enable X register
4. Phase 4 (Zero Page) - First complex addressing mode
5. Phase 6 (Inc/Dec) - Complete basic instruction set
6. Phase 5 (Absolute) - Expand addressing (can parallelize with 4/6)
7. Phase 12 (Flags) - Simple, useful for testing
8. Phase 7 (Branches) - Higher complexity, needs signed arithmetic
9. Phase 11 (Jump) - Complete control flow
10. Phase 13 (Compare) - Complete comparison family
11. Phase 8 (Indexed) - More complex addressing
12. Phase 9 (Stack) - Requires SP register
13. Phase 10 (Indirect) - Most complex addressing
14. Phase 14 (BIT) - Special case, lower priority

**Aggressive approach (feature velocity):**
- Combine phases: do 1+2 together, 4+5+6 together
- Implement all addressing modes for an instruction at once
- Higher risk of bugs but faster to usable ISA

## Open Design Questions

### Q1: Signed Arithmetic for Relative Branches

**Resolved:** Added a ProgramCounter `add_offset` control that latches a signed byte from the data bus and applies it during the Process phase. Assembler encodes signed offsets and range-checks branches.

### Q2: Y Register Timing

**Problem:** Y register needed for some addressing modes (IZY) but not critical early.

**Options:**
1. Add Y in Phase 8 alongside X-indexed modes
2. Defer Y to Phase 10 when IZY is needed
3. Skip Y entirely (X-only design)

**Recommendation:** Defer to Phase 10, reassess based on use cases.

### Q3: Stack Pointer Type

**Problem:** SP needs increment/decrement capability.

**Options:**
1. Use Counter<Byte> (has increment, add decrement)
2. Use ByteRegister + ALU for inc/dec
3. Add dedicated increment/decrement controls to ByteRegister

**Recommendation:** Evaluate in Phase 9 based on microcode needs.

### Q4: Status Flag Direct Control

**Problem:** SEC/CLC need to set/reset individual status flags.

**Options:**
1. Add set/reset controls to each Status bit
2. Use StatusRegister write with bitmask
3. Route through ALU result

**Recommendation:** Option 1 (individual controls) - clean and hardware-ish.

### Q5: BIT Instruction Flag Behavior

**Problem:** BIT sets Z from result but N/V from operand bits.

**Options:**
1. Enhance StatusAnalyzer to accept separate data/flag sources
2. Implement in microcode with multiple steps
3. Simplify BIT behavior (non-6502 compatible)

**Recommendation:** Evaluate in Phase 14 based on StatusAnalyzer design.

## Success Criteria

### Per-Phase

- [ ] All new instructions defined in ISA YAML
- [ ] All microcode sequences implemented and validated
- [ ] All ALU operations implemented and unit tested
- [ ] All ASM integration tests pass
- [ ] Full test suite passes (`make test`)
- [ ] No new compiler warnings
- [ ] CI passes
- [ ] Coverage maintained/improved

### Overall (All Phases Complete)

- [ ] ~128 total instructions implemented
- [ ] 10 addressing modes supported
- [ ] All 6502-inspired core functionality present
- [ ] Comprehensive ASM test suite (one test per instruction)
- [ ] All microcode validated and optimized
- [ ] Documentation updated (design.md, ISA reference)

## Risk Assessment

| Phase | Risk Level | Rationale |
|-------|------------|-----------|
| 1 | Low | Extend existing ALU pattern |
| 2 | Low | Extend existing immediate mode |
| 3 | Low | Simple register operations |
| 4 | Medium | New addressing mode pattern |
| 5 | Low | Reuse absolute mode pattern |
| 6 | Low | Combine existing primitives |
| 7 | High | Signed arithmetic, PC manipulation |
| 8 | Medium | Address calculation complexity |
| 9 | High | New register, complex microcode |
| 10 | Very High | Multiple indirections |
| 11 | Low | Simple unconditional jump |
| 12 | Medium | May need HDL changes |
| 13 | Low | Reuse CMP pattern |
| 14 | Medium | Special flag behavior |

## Next Steps

1. Review this plan with project stakeholders
2. Create GitHub issues/milestones for each phase
3. Begin Phase 1 (ALU Core Operations)
4. Update this document as design questions are resolved
