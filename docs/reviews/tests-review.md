# Integration Tests Review

**Status:** Complete for MVP
**Files:** 7 (6 .asm programs, 1 CMakeLists.txt)
**Issues:** 2 low-priority

## Summary

End-to-end integration tests using assembly programs that exercise the full pipeline: assembler → cartridge → simulator.

## Test Programs

| File | Lines | Tests |
|------|-------|-------|
| hlt.asm | 1 | Basic halt |
| nop.asm | 11 | NOP preserves flags |
| crs.asm | 1 | Crash handling |
| lda.asm | 13 | Load + Z/N flags |
| cmp.asm | 14 | Compare + equal/not-equal |
| jeq.asm | 14 | Conditional branching |

## MVP Instruction Coverage

| Instruction | Integration Test | Unit Tests |
|-------------|-----------------|------------|
| HLT | hlt.asm | Yes |
| NOP | nop.asm | Yes |
| CRS | crs.asm | Yes |
| LDA (IMM) | lda.asm | Yes |
| CMP (IMM) | cmp.asm | Yes |
| JEQ (ABS) | jeq.asm | Yes |

**100% MVP instruction coverage**

## Flag Coverage

| Flag | Direct Test | Indirect Test |
|------|-------------|---------------|
| Z (Zero) | Via JEQ | lda.asm, cmp.asm, jeq.asm |
| N (Negative) | None | lda.asm sets N flag |
| C (Carry) | None | Unit tests only |
| V (Overflow) | None | Unit tests only |

## Test Methodology

Tests use control flow for pass/fail:
- Correct behavior → HLT (exit 0)
- Wrong behavior → CRS (crash)
- crs.asm uses --expect-crash flag

## Findings

### L4: Limited Flag Testing
Only Z flag testable via JEQ. N, C, V require additional instructions or direct flag assertion.

### L5: No Backward Jump Tests
All JEQ tests use forward jumps. Should add loop test.

## CMake Integration

```cmake
foreach(asm_file ${ASM_TEST_FILES})
  # Compile .asm → .bin + .json
  # Register test via ctest
  # Use --expect-crash for crs.asm
endforeach()
```
