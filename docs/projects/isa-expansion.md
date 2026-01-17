# ISA Expansion (Overview)

## Goals

- Grow the ISA in coherent batches aligned with hardware blocks.
- Keep microcode and sim behavior consistent with HDL structure.
- Maintain incremental, test-driven progress.

## Status

This is a high-level overview. See **[isa-expansion-plan.md](isa-expansion-plan.md)** for the detailed implementation plan with 14 phases, microcode patterns, and instruction/addressing mode combinations.

## Target ISA

**~128 instructions** across 10 addressing modes, inspired by the 6502 architecture:

- System: HLT, NOP, CRS
- Arithmetic: ADC, SBC, INC, DEC, INX, DEX (+ INY, DEY if Y register added)
- Logic: AND, ORA, EOR
- Shifts/Rotates: ASL, LSR, ROL, ROR
- Loads/Stores: LDA, LDX, STA, STX (+ LDY, STY if Y register added)
- Transfers: TAX, TXA (+ TAY, TYA, TXS, TSX if Y/SP added)
- Comparisons: CMP, CPX (+ CPY if Y register added)
- Branches: BEQ, BNE, BCS, BCC, BMI, BPL, BVS, BVC
- Jumps: JMP, JSR, RTS
- Stack: PHA, PLA, PHP, PLP
- Flags: SEC, CLC, SEI, CLI, SED, CLD, CLV
- Bit Test: BIT

## Addressing Modes

1. **Implied (IMP)** - No operands (already implemented)
2. **Immediate (IMM)** - 1-byte operand (already implemented)
3. **Absolute (ABS)** - 2-byte address (already implemented)
4. **Zero Page (ZP)** - 1-byte address in page 0x00
5. **Relative (REL)** - Signed byte offset for branches
6. **Zero Page,X (ZPX)** - Zero page + X index
7. **Absolute,X (ABX)** - Absolute + X index
8. **Absolute,Y (ABY)** - Absolute + Y index (if Y added)
9. **Indirect (IND)** - Address of address (for JMP)
10. **Indexed Indirect (IZX)** - (ZP + X) -> address
11. **Indirect Indexed (IZY)** - (ZP) -> address, then + Y

## Implementation Phases (Summary)

See [isa-expansion-plan.md](isa-expansion-plan.md) for full details.

1. **ALU Core Operations** - Implement 11 ALU opcodes (ADD, INC, AND, OR, XOR, shifts, DEC)
2. **Immediate Mode Instructions** - ADC, SBC, AND, ORA, EOR, shifts (9 instructions)
3. **Register Transfer** - TAX, TXA, LDX (3 instructions)
4. **Zero Page Addressing** - LDA/STA/LDX/STX + ALU ops + shifts (18 variants)
5. **Absolute Addressing** - Expand to loads/stores/ALU (14 variants)
6. **Inc/Dec Instructions** - INX, DEX, INC, DEC (6 instructions)
7. **Conditional Branches** - BEQ, BNE, BCS, BCC, BMI, BPL, BVS, BVC (8 instructions)
8. **Indexed Addressing** - ZPX, ABX modes (24 variants)
9. **Stack Operations** - SP register, PHA, PLA, PHP, PLP, JSR, RTS (6 instructions)
10. **Advanced Addressing** - IND, IZX, IZY modes (18 variants)
11. **Jump Instructions** - JMP (2 variants)
12. **Status Flags** - SEC, CLC, SEI, CLI, SED, CLD, CLV (7 instructions)
13. **Comparison Instructions** - CPX, CPY (6 variants)
14. **Bit Test** - BIT (2 variants)

**Total:** ~128 instructions across 14 phases

## Work Pattern

For each phase:

1. **Plan** - Design HDL changes (if needed), microcode patterns, test cases
2. **Implement** - Update ISA YAML, microcode, ALU/sim/HDL as needed
3. **Test** - Add unit tests (ALU) and ASM integration tests (end-to-end)
4. **Validate** - Run validators, check coverage, verify CI
5. **Commit** - Small commits, one per logical change

## Current Priority

**Phase 1: ALU Core Operations** - Implement ADD, INC, AND, OR, XOR, ASL, LSR, ROL, ROR, DEC in the ALU. This unlocks all arithmetic and logic instructions.

## Open Questions

Documented in [isa-expansion-plan.md](isa-expansion-plan.md):

- Signed arithmetic for relative branches
- Y register timing (Phase 8 vs Phase 10)
- Stack pointer type (Counter vs Register)
- Status flag direct control mechanism
- BIT instruction flag behavior
