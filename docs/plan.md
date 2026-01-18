# IRATA2 Next Steps

## Current Status

The core infrastructure is complete. The project has:
- Working assembler, cartridge format, and simulator
- Comprehensive debugging tools (symbols, trace buffer, IPC, dump output)
- Structured logging with failure diagnostics
- Robust microcode validation and optimization (5 validators, 3 optimizers)
- Clean sim architecture with hardware-ish components
- 128-bit control words for future growth

**ISA Implementation:** ~60 instructions across 5 addressing modes (IMP, IMM, ZP, ABS, REL).

## Active Project

**ISA Expansion** - see [docs/projects/isa-expansion.md](projects/isa-expansion.md)

The goal is a 6502-style instruction set. Remaining phases:

| Phase | Description | New Instructions |
|-------|-------------|-----------------|
| 8 | Y register + indexed addressing (ZPX, ZPY, ABX, ABY) + PC refactor | ~56 |
| 9 | Stack operations (SP, PHA, PLA, JSR, RTS) | 8 |
| 10 | Status flag manipulation (CLC, SEC, CLV) | 3 |
| 11 | Compare X/Y and BIT test | 8 |
| 12 | Jump instructions (JMP ABS, JMP IND) | 2 |
| 13 | Indirect indexed addressing (IZX, IZY) - optional | 16 |
| 14 | Interrupts (BRK, RTI) - optional | 2 |

**Next up:** Phase 8 - Y register and indexed addressing modes.

## Optional Future Work

- **Cartridge tools** - see [docs/projects/cartridge-tools.md](projects/cartridge-tools.md)
  - Cartridge inspection CLI
  - Disassembler
  - Round-trip assemble/disassemble tests

## Code Quality

- **Code review checklist** - see [docs/code-review.md](code-review.md)
  - Hardware-ish violation patterns to watch for
  - Known issues to fix

## Development Workflow

For each instruction batch:

1. Update `isa/instructions.yaml` with new instructions
2. Update `microcode/microcode.yaml` with control sequences
3. Add ASM integration tests in `assembler/test/asm_tests/`
4. Run `make test` to verify all tests pass
5. Commit and push, verify CI passes

## Quick Reference

```bash
make configure  # Configure cmake
make build      # Build all
make test       # Run tests
make coverage   # Generate coverage report
```
