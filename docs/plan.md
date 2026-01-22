# IRATA2 Next Steps

## Current Status

The core infrastructure is complete. The project has:
- Working assembler, cartridge format, and simulator
- Comprehensive debugging tools (symbols, trace buffer, IPC, dump output)
- Structured logging with failure diagnostics
- Robust microcode validation and optimization (5 validators, 3 optimizers)
- Clean sim architecture with hardware-ish components
- 128-bit control words for future growth

**ISA Implementation:** 152 instructions across 12 addressing modes (IMP, IMM, ZP, ABS, REL, ZPX, ZPY, ABX, ABY, IND, IZX, IZY).

## Active Project

**ISA Expansion** - completed (see [docs/projects/isa-expansion.md](projects/isa-expansion.md) for the historical plan)

The 6502-style instruction set phases are complete:

| Phase | Description | New Instructions |
|-------|-------------|-----------------|
| 8 | Y register + indexed addressing (ZPX, ZPY, ABX, ABY) + PC refactor | ~56 |
| 9 | Stack operations (SP, PHA, PLA, JSR, RTS) | 8 |
| 10 | Status flag manipulation (CLC, SEC, CLV) | 3 |
| 11 | Compare X/Y and BIT test | 8 |
| 12 | Jump instructions (JMP ABS, JMP IND) | 2 |
| 13 | Indirect indexed addressing (IZX, IZY) - optional | 16 |
| 14 | Interrupts (BRK, RTI) - optional | 2 |

**Next up:** Fix bus validator, then optional future work.

## Urgent: Bus Validator Rewrite

The bus validator (`microcode/src/compiler/bus_validator.cpp`) uses fragile name-based string matching to detect bus conflicts. This approach broke when new controls were added and is now failing to catch real conflicts (e.g., `memory.write` + `status.write` in PLP microcode).

**Problem:** The current implementation has hardcoded component names and pattern matching that doesn't scale and misses conflicts.

**Solution:** Rewrite to use HDL traversal instead of string matching:

1. **Remove all name-based lookups** - Delete the `AnalyzeControl()` function and its hardcoded component lists
2. **Traverse HDL tree** - Walk the HDL component tree to discover all controls automatically
3. **Use control metadata** - Each control knows:
   - Whether it's a ReadControl or WriteControl (from its type)
   - Which bus it's connected to (from its template parameter/parent)
4. **Build bus membership map** - Create a map from ControlInfo* to (bus, read/write) at construction time
5. **Validate using map** - Look up each control in the step and check for conflicts

**Tests to add:**
- Regression test for `memory.write` + `status.write` conflict (the current bug)
- Test that all controls are discovered (compare count against HDL traversal)
- Test for address bus conflicts (e.g., `pc.write` + `tmp.write`)

**Files to modify:**
- `microcode/src/compiler/bus_validator.cpp` - Complete rewrite
- `microcode/include/irata2/microcode/compiler/bus_validator.h` - Add HDL reference to constructor
- `microcode/test/bus_validator_test.cpp` - Add regression tests

## Broken Tests (Blocked on Bus Validator)

Four asm integration tests are failing due to microcode bugs that the bus validator should have caught:

| Test | Issue | Root Cause |
|------|-------|------------|
| `asm_php` | PLP doesn't restore status register | `status.read` is a WriteControl (writes TO bus), but PLP needs to write FROM bus TO status. Need to investigate what control exists for this. |
| `asm_plp` | Same as asm_php | Same issue - PLP microcode uses wrong control direction. |
| `asm_jsr` | JSR jumps to stack address (0x01FE) instead of subroutine | MAR is loaded with target address, then overwritten by stack push operations. Final step copies MAR (now stack addr) to PC instead of target. |
| `asm_rts` | RTS likely has similar issues | Depends on JSR pushing correct return address, which it doesn't. |

**JSR Analysis:**
The JSR_ABS microcode reads the 2-byte target into MAR, then uses MAR for stack pushes (which overwrites it), then copies MAR to PC. The final MAR value is the stack address, not the target.

Options to fix:
1. Use TMP register to save target address (but TMP is a WordRegister on address bus without low/high byte controls)
2. Restructure microcode to push return address first, then read target (like real 6502)
3. Add low/high byte sub-registers to TMP

**PLP/PHP Analysis:**
The status register needs a control that reads FROM the data bus INTO the status register. Currently:
- `status.write` - WriteControl, writes status value TO bus
- `status.read` - Also appears to be a WriteControl based on bus validator error

Need to check HDL for `status.read` control type and add appropriate control if missing.

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
