# ISA Expansion - Step-by-Step Implementation Guide

## Overview

This guide provides detailed, concrete steps for implementing each phase of the ISA expansion plan. It includes safety measures to prevent infinite loops, test infrastructure, and a proven workflow for adding instructions.

**CRITICAL SAFETY RULE**: Always use `max_cycles` parameter in integration tests to prevent infinite loops!

## Status

- Phase 1: ✓ COMPLETE (ALU Core Operations)
- Phase 2: NEXT (Immediate Mode Instructions)

## Table of Contents

1. [Safety Infrastructure](#safety-infrastructure)
2. [Test Infrastructure](#test-infrastructure)
3. [Implementation Workflow](#implementation-workflow)
4. [Phase 2 Example: Immediate Mode Instructions](#phase-2-example)
5. [Common Pitfalls](#common-pitfalls)
6. [Debugging Guide](#debugging-guide)

---

## Safety Infrastructure

### Max Cycles for Integration Tests

**Problem**: Integration tests that run ASM code can enter infinite loops if:
- Microcode is incorrect
- Branch instructions loop forever
- PC doesn't increment properly

**Solution**: Always use `RunUntilHalt(max_cycles)` with a reasonable timeout.

#### Default Max Cycles Guidelines

| Test Type | Recommended Max Cycles | Rationale |
|-----------|----------------------|-----------|
| Simple (HLT, NOP) | 10 | 1-2 cycles per instruction |
| Arithmetic (ADD, SUB) | 100 | Multiple operations |
| Load/Store | 100 | Memory access overhead |
| Branches | 1000 | May loop several times |
| Complex (JSR, RTS) | 10000 | Stack operations, multiple steps |

#### Integration Test Helper

Create `assembler/test/integration_test_helpers.h`:

```cpp
#ifndef IRATA2_ASSEMBLER_TEST_INTEGRATION_TEST_HELPERS_H
#define IRATA2_ASSEMBLER_TEST_INTEGRATION_TEST_HELPERS_H

#include "irata2/assembler/assembler.h"
#include "irata2/sim/cpu.h"
#include <gtest/gtest.h>

namespace irata2::assembler::test {

/**
 * @brief Run assembled code with safety timeout.
 *
 * @param asm_code Assembly source code
 * @param max_cycles Maximum cycles before timeout (default: 1000)
 * @param expect_halt If true, expects normal halt; if false, expects crash
 * @return Final CPU state
 */
inline sim::Cpu::RunResult RunAsm(
    const std::string& asm_code,
    uint64_t max_cycles = 1000,
    bool expect_halt = true) {

  // Assemble the code
  auto result = Assemble(asm_code, "test.asm");

  // Create CPU with assembled ROM
  sim::Cpu cpu(
      sim::DefaultHdl(),
      sim::DefaultMicrocode(),
      std::vector<base::Byte>(result.rom.begin(), result.rom.end()));

  // Set entry point
  cpu.pc().set_value(base::Word{0x8000});

  // Run with timeout
  auto run_result = cpu.RunUntilHalt(max_cycles, /*capture_state=*/true);

  // Verify didn't timeout
  if (run_result.reason == sim::Cpu::HaltReason::Timeout) {
    ADD_FAILURE() << "Test timed out after " << max_cycles
                  << " cycles. Possible infinite loop!\n"
                  << "ASM code:\n" << asm_code;
  }

  // Verify halt vs crash
  if (expect_halt) {
    EXPECT_EQ(run_result.reason, sim::Cpu::HaltReason::Halt)
        << "Expected normal halt, got crash";
  } else {
    EXPECT_EQ(run_result.reason, sim::Cpu::HaltReason::Crash)
        << "Expected crash, got normal halt";
  }

  return run_result;
}

/**
 * @brief Run assembled code and check final register values.
 */
inline void RunAsmAndCheckRegisters(
    const std::string& asm_code,
    std::optional<base::Byte> expected_a = std::nullopt,
    std::optional<base::Byte> expected_x = std::nullopt,
    std::optional<base::Byte> expected_status = std::nullopt,
    uint64_t max_cycles = 1000) {

  auto result = RunAsm(asm_code, max_cycles);

  ASSERT_TRUE(result.state.has_value()) << "No CPU state captured";

  if (expected_a.has_value()) {
    EXPECT_EQ(result.state->a, *expected_a)
        << "A register mismatch";
  }

  if (expected_x.has_value()) {
    EXPECT_EQ(result.state->x, *expected_x)
        << "X register mismatch";
  }

  if (expected_status.has_value()) {
    EXPECT_EQ(result.state->status, *expected_status)
        << "Status register mismatch";
  }
}

/**
 * @brief Run assembled code and check status flags.
 */
inline void RunAsmAndCheckFlags(
    const std::string& asm_code,
    bool expect_zero = false,
    bool expect_negative = false,
    bool expect_carry = false,
    bool expect_overflow = false,
    uint64_t max_cycles = 1000) {

  auto result = RunAsm(asm_code, max_cycles);
  ASSERT_TRUE(result.state.has_value());

  base::Byte status = result.state->status;

  EXPECT_EQ((status.value() & 0x01) != 0, expect_zero)
      << "Zero flag mismatch";
  EXPECT_EQ((status.value() & 0x02) != 0, expect_negative)
      << "Negative flag mismatch";
  EXPECT_EQ((status.value() & 0x04) != 0, expect_carry)
      << "Carry flag mismatch";
  EXPECT_EQ((status.value() & 0x08) != 0, expect_overflow)
      << "Overflow flag mismatch";
}

}  // namespace irata2::assembler::test

#endif  // IRATA2_ASSEMBLER_TEST_INTEGRATION_TEST_HELPERS_H
```

---

## Test Infrastructure

### Directory Structure

Create the following structure for integration tests:

```
assembler/test/
├── CMakeLists.txt
├── assembler_test.cpp                    # Basic assembler unit tests
├── integration_test_helpers.h            # NEW: Helper utilities
└── integration/                          # NEW: Integration tests by instruction
    ├── system_test.cpp                   # HLT, NOP, CRS
    ├── load_immediate_test.cpp           # LDA_IMM
    ├── arithmetic_immediate_test.cpp     # ADC_IMM, SBC_IMM (Phase 2)
    ├── logic_immediate_test.cpp          # AND_IMM, ORA_IMM, EOR_IMM (Phase 2)
    └── shift_accumulator_test.cpp        # ASL_IMP, LSR_IMP, ROL_IMP, ROR_IMP (Phase 2)
```

### CMakeLists.txt Updates

Add to `assembler/test/CMakeLists.txt`:

```cmake
# Integration tests - run assembled programs end-to-end
# These tests run actual CPU simulations and could hang if microcode is buggy
add_executable(integration_tests
    integration/system_test.cpp
    integration/load_immediate_test.cpp
    # Add more as you implement them
)

target_link_libraries(integration_tests PRIVATE
    irata2::assembler
    irata2::sim
    irata2::base
    GTest::gtest_main
)

target_include_directories(integration_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Set CTest timeout - kills hung tests even if max_cycles fails
gtest_discover_tests(integration_tests
    PROPERTIES TIMEOUT 60  # Longer timeout for integration tests
)
```

**Multiple layers of timeout protection:**
1. **Code level**: `max_cycles` parameter in tests (prevents infinite loops in microcode)
2. **CTest level**: `TIMEOUT` property (kills hung test processes)
3. **Manual**: Ctrl+C if needed (last resort)

**Timeout recommendations:**
- Unit tests: 30 seconds
- Integration tests: 60 seconds
- Can be overridden per-test with `set_tests_properties()` if needed

### Example Integration Test

`assembler/test/integration/system_test.cpp`:

```cpp
#include "../integration_test_helpers.h"

using namespace irata2::assembler::test;
using namespace irata2::base;

TEST(SystemIntegrationTest, HaltStopsCpu) {
  RunAsm("HLT\n", /*max_cycles=*/10);
  // If we get here without timeout, test passed
}

TEST(SystemIntegrationTest, NopDoesNothing) {
  auto result = RunAsm(R"(
    NOP
    NOP
    NOP
    HLT
  )", /*max_cycles=*/20);

  EXPECT_LE(result.cycles, 10);  // Should be ~4 cycles
}

TEST(SystemIntegrationTest, CrashStopsCpu) {
  RunAsm("CRS\n", /*max_cycles=*/10, /*expect_halt=*/false);
}
```

`assembler/test/integration/load_immediate_test.cpp`:

```cpp
#include "../integration_test_helpers.h"

using namespace irata2::assembler::test;
using namespace irata2::base;

TEST(LoadImmediateTest, LdaLoadsValue) {
  RunAsmAndCheckRegisters(R"(
    LDA #$42
    HLT
  )", /*expected_a=*/Byte{0x42});
}

TEST(LoadImmediateTest, LdaSetsZeroFlag) {
  RunAsmAndCheckFlags(R"(
    LDA #$00
    HLT
  )", /*expect_zero=*/true);
}

TEST(LoadImmediateTest, LdaSetsNegativeFlag) {
  RunAsmAndCheckFlags(R"(
    LDA #$80
    HLT
  )", /*expect_negative=*/true);
}
```

---

## Implementation Workflow

### For Each New Instruction

Follow these steps IN ORDER:

#### Step 1: Update ISA YAML

File: `isa/instructions.yaml`

```yaml
- mnemonic: ADC        # Instruction name
  opcode: 0x20         # Unique opcode (check existing, increment)
  addressing_mode: IMM # One of: IMP, IMM, ABS, ZP, etc.
  cycles: 2            # Expected cycles (will verify in test)
  description: "Add with carry (immediate)"
  category: Arithmetic # For documentation
  flags_affected: [Z, N, C, V]  # Which flags change
```

**IMPORTANT**: Choose opcode carefully! Check existing opcodes:
```bash
grep "opcode:" isa/instructions.yaml | sort
```

#### Step 2: Write Microcode

File: `microcode/microcode.yaml`

Add the microcode sequence. Pattern for `ADC_IMM`:

```yaml
ADC_IMM:
  stages:
    - steps:
        # Fetch immediate operand
        - [pc.write, memory.mar.read]
        - [memory.write, alu.rhs.read, pc.increment]

        # Load A into ALU LHS
        - [a.write, alu.lhs.read]

        # Set ALU opcode for ADD (0x1 = bit 0 only)
        - [alu.opcode_bit_0]

        # Write ALU result back to A and update status
        - [alu.result.write, a.read, status.analyzer.read]
```

**Microcode Patterns Reference**:

- **Fetch immediate byte**:
  ```yaml
  - [pc.write, memory.mar.read]
  - [memory.write, <destination>.read, pc.increment]
  ```

- **Fetch absolute address** (2 bytes):
  ```yaml
  - [pc.write, memory.mar.read]
  - [memory.write, <dest>.low.read, pc.increment]
  - [pc.write, memory.mar.read]
  - [memory.write, <dest>.high.read, pc.increment]
  ```

- **ALU operation**:
  ```yaml
  - [<input1>.write, alu.lhs.read]
  - [<input2>.write, alu.rhs.read]
  - [alu.opcode_bit_0]  # Set opcode bits as needed
  - [alu.opcode_bit_1]  # bit pattern for operation
  - [alu.result.write, <dest>.read, status.analyzer.read]
  ```

**ALU Opcodes** (from Phase 1):
- 0x1 (0001): ADD - `[alu.opcode_bit_0]`
- 0x2 (0010): SUB - `[alu.opcode_bit_1]`
- 0x3 (0011): INC - `[alu.opcode_bit_0, alu.opcode_bit_1]`
- 0x4 (0100): AND - `[alu.opcode_bit_2]`
- 0x5 (0101): OR - `[alu.opcode_bit_0, alu.opcode_bit_2]`
- 0x6 (0110): XOR - `[alu.opcode_bit_1, alu.opcode_bit_2]`
- 0x7 (0111): ASL - `[alu.opcode_bit_0, alu.opcode_bit_1, alu.opcode_bit_2]`
- 0x8 (1000): LSR - `[alu.opcode_bit_3]`
- 0x9 (1001): ROL - `[alu.opcode_bit_0, alu.opcode_bit_3]`
- 0xA (1010): ROR - `[alu.opcode_bit_1, alu.opcode_bit_3]`
- 0xB (1011): DEC - `[alu.opcode_bit_0, alu.opcode_bit_1, alu.opcode_bit_3]`

#### Step 3: Rebuild ISA and Microcode

```bash
cmake --build build --target generate_isa
cmake --build build --parallel
```

This regenerates `build/isa/include/irata2/isa/isa.h` and compiles microcode.

**CRITICAL**: Check for microcode validation errors in the build output!

#### Step 4: Write Integration Test

Create or add to appropriate file in `assembler/test/integration/`.

**Example for ADC_IMM**:

```cpp
TEST(ArithmeticImmediateTest, AdcBasicAddition) {
  RunAsmAndCheckRegisters(R"(
    LDA #$10   ; A = 0x10
    ADC #$05   ; A = 0x10 + 0x05 = 0x15
    HLT
  )", /*expected_a=*/Byte{0x15}, std::nullopt, std::nullopt, /*max_cycles=*/100);
}

TEST(ArithmeticImmediateTest, AdcWithCarry) {
  // Manually set carry flag (will need SEC instruction eventually)
  // For now, test carry from previous operation
  RunAsmAndCheckRegisters(R"(
    LDA #$FF   ; A = 0xFF
    ADC #$02   ; A = 0xFF + 0x02 + 0 = 0x01, C=1
    ADC #$00   ; A = 0x01 + 0x00 + 1 = 0x02, C=0
    HLT
  )", /*expected_a=*/Byte{0x02}, std::nullopt, std::nullopt, /*max_cycles=*/100);
}

TEST(ArithmeticImmediateTest, AdcSetsZeroFlag) {
  RunAsmAndCheckFlags(R"(
    LDA #$00
    ADC #$00
    HLT
  )", /*expect_zero=*/true, false, false, false, /*max_cycles=*/100);
}

TEST(ArithmeticImmediateTest, AdcSetsNegativeFlag) {
  RunAsmAndCheckFlags(R"(
    LDA #$7F   ; 127
    ADC #$01   ; 127 + 1 = 128 = 0x80 (negative in signed)
    HLT
  )", false, /*expect_negative=*/true, false, false, /*max_cycles=*/100);
}

TEST(ArithmeticImmediateTest, AdcSetsCarryFlag) {
  RunAsmAndCheckFlags(R"(
    LDA #$FF   ; 255
    ADC #$01   ; 255 + 1 = 256 (overflow, C=1, result=0)
    HLT
  )", /*expect_zero=*/true, false, /*expect_carry=*/true, false, /*max_cycles=*/100);
}

TEST(ArithmeticImmediateTest, AdcSetsOverflowFlag) {
  RunAsmAndCheckFlags(R"(
    LDA #$7F   ; +127 (positive)
    ADC #$01   ; +127 + 1 = +128 -> 0x80 (-128, signed overflow)
    HLT
  )", false, true, false, /*expect_overflow=*/true, /*max_cycles=*/100);
}
```

#### Step 5: Run Tests (SAFELY)

**NEVER run tests without max_cycles!**

```bash
# Build with tests
cmake -B build -DBUILD_TESTING=ON
cmake --build build --parallel

# Run just your new test file
./build/assembler/test/integration_tests --gtest_filter="ArithmeticImmediateTest.*"

# If it hangs, you have an infinite loop! Ctrl+C and debug microcode
```

#### Step 6: Debug If Needed

If tests timeout or fail:

1. **Check microcode validation output**:
   ```bash
   cmake --build build 2>&1 | grep -i "error\|warning"
   ```

2. **Decode the microcode**:
   ```bash
   ./build/microcode/microcode_dump_main \
       --input build/microcode/microcode.bin \
       --output debug_microcode.yaml
   grep -A 20 "ADC_IMM" debug_microcode.yaml
   ```

3. **Enable debug logging** (if test runs but gives wrong result):
   ```bash
   IRATA2_LOG_LEVEL=debug ./build/assembler/test/integration_tests \
       --gtest_filter="ArithmeticImmediateTest.AdcBasicAddition"
   ```

4. **Check ALU unit tests** (if arithmetic is wrong):
   ```bash
   ./build/sim/test/sim_tests --gtest_filter="AluTest.Add*"
   ```

#### Step 7: Commit

Once tests pass:

```bash
git add -A
git commit -m "Phase 2.1: Implement ADC immediate mode instruction

- Add ADC_IMM to isa/instructions.yaml (opcode 0x20)
- Add microcode sequence for immediate mode addition with carry
- Add integration tests with flag verification
- All tests pass (6/6)"

# Push with branch name matching session ID
git push -u origin claude/isa-expansion-plan-CpbCk
```

---

## Phase 2 Example

### Phase 2: Immediate Mode Instructions

**Goal**: Add arithmetic, logic, and shift instructions using immediate/implied modes.

**Batches**:
1. Arithmetic: ADC, SBC (2 instructions)
2. Logic: AND, ORA, EOR (3 instructions)
3. Shifts: ASL, LSR, ROL, ROR (4 instructions, accumulator mode)

**Total**: 9 instructions

---

### Phase 2.1: Arithmetic Instructions

#### Instructions to Add

| Mnemonic | Opcode | Mode | Description |
|----------|--------|------|-------------|
| ADC | 0x20 | IMM | Add with carry |
| SBC | 0x21 | IMM | Subtract with carry |

#### ISA YAML Entry

```yaml
# Add after existing instructions in isa/instructions.yaml

- mnemonic: ADC
  opcode: 0x20
  addressing_mode: IMM
  cycles: 2
  description: "Add with carry (immediate)"
  category: Arithmetic
  flags_affected: [Z, N, C, V]

- mnemonic: SBC
  opcode: 0x21
  addressing_mode: IMM
  cycles: 2
  description: "Subtract with carry (immediate)"
  category: Arithmetic
  flags_affected: [Z, N, C, V]
```

#### Microcode YAML Entry

```yaml
# Add to microcode/microcode.yaml

ADC_IMM:
  stages:
    - steps:
        - [pc.write, memory.mar.read]
        - [memory.write, alu.rhs.read, pc.increment]
        - [a.write, alu.lhs.read]
        - [alu.opcode_bit_0]  # ADD opcode
        - [alu.result.write, a.read, status.analyzer.read]

SBC_IMM:
  stages:
    - steps:
        - [pc.write, memory.mar.read]
        - [memory.write, alu.rhs.read, pc.increment]
        - [a.write, alu.lhs.read]
        - [alu.opcode_bit_1]  # SUB opcode
        - [alu.result.write, a.read, status.analyzer.read]
```

#### Integration Tests

Create `assembler/test/integration/arithmetic_immediate_test.cpp`:

```cpp
#include "../integration_test_helpers.h"

using namespace irata2::assembler::test;
using namespace irata2::base;

// ADC tests
TEST(ArithmeticImmediateTest, AdcBasicAddition) {
  RunAsmAndCheckRegisters(R"(
    LDA #$10
    ADC #$05
    HLT
  )", Byte{0x15}, std::nullopt, std::nullopt, 100);
}

TEST(ArithmeticImmediateTest, AdcWithOverflow) {
  RunAsmAndCheckFlags(R"(
    LDA #$FF
    ADC #$01
    HLT
  )", true, false, true, false, 100);
}

TEST(ArithmeticImmediateTest, AdcSetsNegative) {
  RunAsmAndCheckFlags(R"(
    LDA #$7F
    ADC #$01
    HLT
  )", false, true, false, true, 100);
}

// SBC tests
TEST(ArithmeticImmediateTest, SbcBasicSubtraction) {
  RunAsmAndCheckRegisters(R"(
    LDA #$10
    SBC #$05
    HLT
  )", Byte{0x0B}, std::nullopt, std::nullopt, 100);
}

TEST(ArithmeticImmediateTest, SbcWithBorrow) {
  RunAsmAndCheckFlags(R"(
    LDA #$00
    SBC #$01
    HLT
  )", false, true, false, false, 100);
}
```

#### Build and Test

```bash
# Rebuild
cmake --build build --target generate_isa
cmake --build build --parallel

# Test
./build/assembler/test/integration_tests --gtest_filter="ArithmeticImmediateTest.*"
```

#### Commit

```bash
git add -A
git commit -m "Phase 2.1: Add ADC and SBC immediate mode instructions"
git push -u origin claude/isa-expansion-plan-CpbCk
```

---

### Phase 2.2: Logic Instructions

Repeat the same process for AND, ORA, EOR.

**Opcodes**: 0x22 (AND), 0x23 (ORA), 0x24 (EOR)

**ALU opcodes**: 0x4 (AND), 0x5 (OR), 0x6 (XOR)

**File**: `assembler/test/integration/logic_immediate_test.cpp`

---

### Phase 2.3: Shift Instructions (Accumulator Mode)

**Special case**: These operate on the accumulator (implied mode), not immediate operands.

**Opcodes**: 0x25 (ASL), 0x26 (LSR), 0x27 (ROL), 0x28 (ROR)

**Addressing Mode**: IMP (implied)

**Microcode pattern**:

```yaml
ASL_IMP:
  stages:
    - steps:
        - [a.write, alu.lhs.read]
        - [alu.opcode_bit_0, alu.opcode_bit_1, alu.opcode_bit_2]  # 0x7
        - [alu.result.write, a.read, status.analyzer.read]
```

---

## Common Pitfalls

### 1. Forgetting max_cycles

**DON'T**:
```cpp
auto result = cpu.RunUntilHalt();  // INFINITE LOOP RISK!
```

**DO**:
```cpp
auto result = cpu.RunUntilHalt(1000);  // Safe
```

### 2. Wrong ALU Opcode Bits

**Problem**: Setting wrong bits gives wrong operation.

**Example**: Want ADD (0x1), but set `[alu.opcode_bit_1]` → SUB (0x2)

**Solution**: Double-check ALU opcode table in this document.

### 3. Not Incrementing PC

**Problem**: PC doesn't advance → infinite loop on same instruction

**Solution**: Always include `pc.increment` after fetching operands

### 4. Status Flags Not Updated

**Problem**: Forgot `status.analyzer.read` in microcode

**Solution**: Always end ALU operations with:
```yaml
- [alu.result.write, <dest>.read, status.analyzer.read]
```

### 5. Opcode Conflicts

**Problem**: Two instructions with same opcode

**Solution**: Always check existing opcodes before adding new one:
```bash
grep "opcode:" isa/instructions.yaml | sort -t: -k2 -n
```

### 6. Running Full Test Suite Too Early

**Problem**: Running all tests when only one instruction is implemented

**Solution**: Use gtest filters:
```bash
./build/assembler/test/integration_tests --gtest_filter="ArithmeticImmediateTest.*"
```

---

## Debugging Guide

### When Tests Timeout (Infinite Loop)

1. **Stop the test** (Ctrl+C)

2. **Check microcode is valid**:
   ```bash
   cmake --build build 2>&1 | grep -A5 "microcode"
   ```

3. **Decode microcode**:
   ```bash
   ./build/microcode/microcode_dump_main \
       --input build/microcode/microcode.bin \
       --output debug_microcode.yaml
   ```

4. **Look for**:
   - Missing `pc.increment`
   - Wrong opcode in instruction table
   - Microcode sequence name mismatch

### When Tests Fail (Wrong Results)

1. **Enable detailed logging**:
   ```bash
   IRATA2_LOG_LEVEL=trace ./build/assembler/test/integration_tests \
       --gtest_filter="YourTest.Name"
   ```

2. **Check ALU is working**:
   ```bash
   ./build/sim/test/sim_tests --gtest_filter="AluTest.*"
   ```

3. **Verify microcode sequence**:
   - Check step-by-step execution
   - Verify ALU opcode bits match expected operation
   - Check status.analyzer.read is present

### When Build Fails

1. **ISA generation errors**:
   ```bash
   cmake --build build --target generate_isa -- VERBOSE=1
   ```

2. **Microcode compilation errors**:
   - Check YAML syntax
   - Check control names match HDL
   - Check variant conditions are valid

---

## Next Steps

### Immediate Actions

1. **Create test infrastructure**:
   - [ ] Create `assembler/test/integration_test_helpers.h`
   - [ ] Create `assembler/test/integration/` directory
   - [ ] Update `assembler/test/CMakeLists.txt`
   - [ ] Write baseline tests for existing instructions (HLT, NOP, LDA, CMP)

2. **Implement Phase 2.1**:
   - [ ] Add ADC_IMM and SBC_IMM to isa/instructions.yaml
   - [ ] Add microcode sequences
   - [ ] Write integration tests
   - [ ] Verify all tests pass
   - [ ] Commit and push

3. **Continue with Phase 2.2 and 2.3**:
   - Follow same pattern for logic and shift instructions

### Long-term Plan

After Phase 2 complete, proceed with:
- Phase 3: Register transfers (TAX, TXA, LDX)
- Phase 4: Zero page addressing mode
- Phase 5: Absolute addressing expansion
- ... (see isa-expansion-plan.md for full roadmap)

---

## Success Criteria

Before moving to next phase:

- [ ] All instructions added to ISA YAML
- [ ] All microcode sequences validated
- [ ] All integration tests pass with max_cycles safety
- [ ] No timeouts or infinite loops
- [ ] Full test suite passes: `make test`
- [ ] CI passes after push
- [ ] Code committed with clear messages

---

## CTest Timeout Configuration

### Multiple Layers of Protection

The project uses **three layers** of timeout protection to prevent infinite loops:

#### 1. Code-Level Timeouts (Primary Defense)

```cpp
// Every test MUST specify max_cycles
RunAsm("LDA #$42\nHLT\n", /*max_cycles=*/100);
```

This is checked in the test code itself and will fail the test gracefully with a helpful error message.

#### 2. CTest-Level Timeouts (Secondary Defense)

Set in CMakeLists.txt via `gtest_discover_tests()`:

```cmake
gtest_discover_tests(integration_tests
    PROPERTIES TIMEOUT 60  # Kills process after 60 seconds
)
```

This kills the entire test process if it hangs, even if the code-level timeout fails.

#### 3. Global CTest Timeout (Tertiary Defense)

Can also set a global timeout for ALL tests:

```bash
# In CMakeLists.txt (top-level or test directory)
set(CTEST_TEST_TIMEOUT 120)  # Global default: 120 seconds

# Or via command line
ctest --timeout 120
```

### Timeout Best Practices

| Test Type | max_cycles | TIMEOUT | Rationale |
|-----------|-----------|---------|-----------|
| Unit tests (sim/test) | N/A | 30s | Fast, deterministic |
| Integration tests (assembler) | 100-1000 | 60s | Runs microcode, needs headroom |
| Complex integration | 10000 | 120s | Stack/JSR/RTS operations |

### Per-Test Timeout Override

For specific slow tests, override individually:

```cmake
gtest_discover_tests(integration_tests
    PROPERTIES TIMEOUT 60
)

# After discovery, override specific tests
set_tests_properties(
    "IntegrationTest.ComplexStackOperations"
    PROPERTIES TIMEOUT 120
)
```

### Debugging Timeout Failures

When a test times out:

1. **CTest timeout** → Process killed, no output
   ```
   Test #42: IntegrationTest.Foo .........***Timeout
   ```
   **Fix**: Increase `TIMEOUT` or investigate infinite loop in microcode

2. **Code timeout** → Test fails gracefully with details
   ```
   Test timed out after 1000 cycles. Possible infinite loop!
   ASM code: ...
   ```
   **Fix**: Check microcode for missing `pc.increment` or wrong branch logic

### Running Tests with Custom Timeout

```bash
# Override all timeouts to 10 seconds (for quick feedback)
ctest --timeout 10

# Run specific test with verbose output
ctest -R IntegrationTest.Foo -VV --timeout 30

# Run all integration tests with longer timeout
ctest -R ".*IntegrationTest.*" --timeout 120
```

## Quick Reference

### Test Helper Functions

```cpp
// Run ASM with timeout
RunAsm(asm_code, max_cycles, expect_halt)

// Run and check registers
RunAsmAndCheckRegisters(asm_code, expected_a, expected_x, expected_status, max_cycles)

// Run and check flags
RunAsmAndCheckFlags(asm_code, zero, negative, carry, overflow, max_cycles)
```

### ALU Opcodes

| Op | Hex | Bits | Microcode |
|----|-----|------|-----------|
| ADD | 0x1 | 0001 | `[alu.opcode_bit_0]` |
| SUB | 0x2 | 0010 | `[alu.opcode_bit_1]` |
| INC | 0x3 | 0011 | `[alu.opcode_bit_0, alu.opcode_bit_1]` |
| AND | 0x4 | 0100 | `[alu.opcode_bit_2]` |
| OR  | 0x5 | 0101 | `[alu.opcode_bit_0, alu.opcode_bit_2]` |
| XOR | 0x6 | 0110 | `[alu.opcode_bit_1, alu.opcode_bit_2]` |
| ASL | 0x7 | 0111 | `[alu.opcode_bit_0, alu.opcode_bit_1, alu.opcode_bit_2]` |
| LSR | 0x8 | 1000 | `[alu.opcode_bit_3]` |
| ROL | 0x9 | 1001 | `[alu.opcode_bit_0, alu.opcode_bit_3]` |
| ROR | 0xA | 1010 | `[alu.opcode_bit_1, alu.opcode_bit_3]` |
| DEC | 0xB | 1011 | `[alu.opcode_bit_0, alu.opcode_bit_1, alu.opcode_bit_3]` |

### Microcode Patterns

```yaml
# Fetch immediate
- [pc.write, memory.mar.read]
- [memory.write, <dest>.read, pc.increment]

# ALU operation
- [<src1>.write, alu.lhs.read]
- [<src2>.write, alu.rhs.read]
- [alu.opcode_bit_X, ...]  # Set opcode bits
- [alu.result.write, <dest>.read, status.analyzer.read]
```

### Build Commands

```bash
# Full rebuild
cmake --build build --target generate_isa
cmake --build build --parallel

# Run specific test
./build/assembler/test/integration_tests --gtest_filter="Test.Name"

# Run with logging
IRATA2_LOG_LEVEL=debug ./build/assembler/test/integration_tests

# Check coverage
make coverage
```
