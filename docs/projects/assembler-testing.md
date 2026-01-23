# Assembler Testing and Modularity Improvements

## Goal

Enable modular assembly development with comprehensive testing infrastructure to make demo programs easier to develop, test, and debug.

## Motivation

**Current Pain Points:**
- Asteroids demo is 175 lines in single file (hard to understand and modify)
- All memory addresses are magic numbers (`$00`, `$4100`, etc.)
- No way to test individual subroutines in isolation
- Graphics output must be verified manually (despite having ImageBackend!)
- No code reuse between demos (each must reimplement VGC wrapper code)

**Desired State:**
- Demos organized into logical modules (graphics.asm, input.asm, physics.asm)
- Named constants for all addresses (SHIP_X, VGC_CMD, COLOR_WHITE)
- Automated tests for graphics subroutines (verify pixels drawn)
- Test-driven development workflow for assembly code
- Shared library code across demos

## Implementation Plan

### Phase 1: File Inclusion and Named Constants

**Timeline:** 3-4 days

#### 1.1 Add String Literal Tokens (Lexer)

**Changes to `assembler/include/irata2/assembler/token.h`:**
```cpp
enum class TokenKind {
  // ... existing ...
  String,  // "file.asm"
};

struct Token {
  // ... existing ...
  std::optional<std::string> string_value;
};
```

**Changes to `assembler/src/lexer.cpp`:**
- Add `Lexer::LexStringLiteral()` method
- Handle `"` character in main lex loop
- Support escape sequences: `\"` and `\\`

**Tests:**
- String literal parsing
- Escaped quotes and backslashes
- Unterminated string error
- Empty string

#### 1.2 Add `.include` Directive (Parser)

**Changes to `assembler/include/irata2/assembler/ast.h`:**
```cpp
struct DirectiveStmt {
  enum class Type { Org, Byte, Include };
  // ... existing fields ...
  std::optional<std::string> include_path;  // For Include directive
};
```

**Changes to `assembler/src/parser.cpp`:**
- Recognize `.include` directive
- Parse string literal operand
- Recursively parse included file
- Merge included statements into current program
- Track include stack for cycle detection

**Implementation Details:**
```cpp
class Parser {
 private:
  std::string current_dir_;  // Directory of current file
  std::set<std::string> include_stack_;  // For cycle detection

  void ParseIncludeDirective();
  std::string ResolveIncludePath(const std::string& path);
};
```

**Tests:**
- Include single file
- Include multiple files
- Nested includes (A→B→C)
- Circular include detection (A→B→A)
- Label resolution across files
- Error spans reference correct files
- Relative path resolution

#### 1.3 Add `.equ` Directive (Assembler)

**Changes to `assembler/src/assembler.cpp`:**
```cpp
// Separate symbol table for constants (not addresses!)
std::unordered_map<std::string, uint32_t> constants_;

// First pass: collect .equ definitions before processing instructions
void CollectConstants(const Program& program);

// Operand resolution: check constants table
uint32_t ResolveConstantOrLabel(const Operand& op, const SymbolTable& symbols);
```

**Syntax:**
```asm
.equ NAME, value
.equ SHIP_X, $00
.equ VGC_CMD, $4100
```

**Tests:**
- Define and use constants
- Constants in all addressing modes
- Forward reference to constants
- Duplicate constant name error
- Constant value out of range

**Deliverables:**
- [ ] String literal lexer implementation
- [ ] `.include` directive parser and tests
- [ ] `.equ` directive implementation and tests
- [ ] Integration test: multi-file program with constants
- [ ] Documentation update in assembly-guide.md

---

### Phase 2: Graphics Testing Framework

**Timeline:** 2 days

#### 2.1 Create Graphics Test Helpers

**New file: `assembler/test/graphics_test_helpers.h`**

```cpp
namespace irata2::assembler::test {

// Execute assembly program and return framebuffer
sim::io::ImageBackend ExecuteAndRender(
    const std::string& source,
    size_t max_cycles = 10000);

// Assertion helpers
void ExpectPixel(const sim::io::ImageBackend& fb,
                 uint8_t x, uint8_t y, uint8_t intensity);

void ExpectLine(const sim::io::ImageBackend& fb,
                uint8_t x0, uint8_t y0,
                uint8_t x1, uint8_t y1,
                uint8_t intensity);

void ExpectClear(const sim::io::ImageBackend& fb,
                 uint8_t intensity);

// Diagnostics
size_t CountPixels(const sim::io::ImageBackend& fb,
                   uint8_t intensity);

std::string DumpFramebuffer(const sim::io::ImageBackend& fb);

}  // namespace
```

**Implementation: `assembler/test/graphics_test_helpers.cpp`**
- Assemble source to cartridge
- Create CPU with ImageBackend VGC
- Run until HLT or max_cycles
- Return ImageBackend for inspection

#### 2.2 Write Graphics Tests

**New file: `assembler/test/graphics_integration_test.cpp`**

```cpp
TEST(GraphicsIntegration, DrawPoint) {
  const char* source = R"(
    .org $8000
    LDA #$02        ; POINT command
    STA $4100
    LDA #$80        ; X=128
    STA $4101
    LDA #$90        ; Y=144
    STA $4102
    LDA #$03        ; Intensity=white
    STA $4105
    LDA #$01        ; Execute
    STA $4106
    HLT
  )";

  auto fb = ExecuteAndRender(source);
  ExpectPixel(fb, 128, 144, 3);
  EXPECT_EQ(CountPixels(fb, 3), 1);
}

TEST(GraphicsIntegration, DrawLine) {
  // Similar test for LINE command
}

TEST(GraphicsIntegration, ClearScreen) {
  // Test CLEAR command fills all pixels
}
```

**Deliverables:**
- [ ] Graphics test helper implementation
- [ ] Tests for all VGC commands (CLEAR, POINT, LINE)
- [ ] Example subroutine test (draw triangle)
- [ ] Documentation in docs/testing-guide.md

---

### Phase 3: Assembly Testing Infrastructure

**Timeline:** 3-4 days

#### Approach: Macro-Based Testing (No Simulator Changes)

**Rationale:** Pure software approach, no new opcodes or MMIO devices needed.

#### 3.1 Add Testing Macros

**New file: `demos/lib/test_macros.asm`**

```asm
; Test accumulator value
; Usage: .test_a $42, "A should be 0x42"
; Expands to:
;   CMP #$42
;   BEQ .test_pass_N
;   CRS              ; Crash if mismatch
; .test_pass_N:

; Test X register value
; .test_x value, "description"

; Test Y register value
; .test_y value, "description"

; Test memory value
; .test_mem addr, value, "description"
```

**Implementation:** These are just commented patterns for now. Later can add macro preprocessor.

#### 3.2 Create AssemblyTestRunner

**New file: `assembler/test/assembly_test_runner.h`**

```cpp
namespace irata2::assembler::test {

struct TestResult {
  bool passed;
  std::string test_name;
  std::optional<std::string> failure_message;
  size_t cycles_executed;
};

class AssemblyTestRunner {
 public:
  // Run .asm file and return test results
  std::vector<TestResult> RunTests(const std::string& asm_file);

  // Run and expect all tests pass (throws on failure)
  void ExpectAllPass(const std::string& asm_file);

  // Run single test and expect pass
  void ExpectTestPass(const std::string& source,
                      const std::string& test_name);
};

}  // namespace
```

**Implementation:**
- Assemble source
- Run on CPU
- Detect test failure (HLT with CRS or specific pattern)
- Parse test descriptions from debug symbols
- Return detailed results

#### 3.3 Example Assembly Tests

**New file: `demos/asteroids/tests/graphics_test.asm`**

```asm
; Test drawing ship triangle

.include "../lib/constants.asm"
.include "../graphics.asm"

.org $8000
  ; Set ship position
  LDA #$80
  STA SHIP_X
  LDA #$90
  STA SHIP_Y

  ; Clear screen
  JSR clear_screen

  ; Draw ship
  JSR draw_ship

  ; Present and halt
  JSR present_frame
  HLT
```

**Corresponding C++ test:**
```cpp
TEST(AsteroidsGraphics, DrawShip) {
  auto fb = ExecuteAndRender(
      ReadFile("demos/asteroids/tests/graphics_test.asm"));

  // Verify triangle vertices
  ExpectLine(fb, 128, 138, 124, 148, 3);  // Left edge
  ExpectLine(fb, 124, 148, 132, 148, 3);  // Base
  ExpectLine(fb, 132, 148, 128, 138, 3);  // Right edge
}
```

**Deliverables:**
- [ ] AssemblyTestRunner implementation
- [ ] Test macro patterns documented
- [ ] Example tests for asteroids modules
- [ ] Integration with GoogleTest
- [ ] Documentation in docs/testing-guide.md

---

### Phase 4: Refactor Asteroids Demo

**Timeline:** 2-3 days

#### 4.1 Create Module Structure

```
demos/asteroids/
├── main.asm              # Entry point, main loop
├── lib/
│   ├── constants.asm     # All .equ definitions
│   ├── zero_page.asm     # Zero page layout documentation
│   ├── vgc_lib.asm       # VGC wrapper subroutines
│   └── input_lib.asm     # Input wrapper subroutines
├── graphics.asm          # Ship and asteroid drawing
├── input.asm             # Input polling and handling
├── physics.asm           # Movement updates
└── tests/
    ├── graphics_test.asm
    ├── input_test.asm
    └── physics_test.asm
```

#### 4.2 Extract Constants

**`lib/constants.asm`:**
```asm
; Zero page variables
.equ SHIP_X, $00
.equ SHIP_Y, $01

; MMIO devices
.equ INPUT_STATUS, $4000
.equ INPUT_DATA, $4002

.equ VGC_CMD, $4100
.equ VGC_X0, $4101
.equ VGC_Y0, $4102
.equ VGC_X1, $4103
.equ VGC_Y1, $4104
.equ VGC_COLOR, $4105
.equ VGC_EXEC, $4106
.equ VGC_CONTROL, $4107

; VGC commands
.equ CMD_CLEAR, $01
.equ CMD_POINT, $02
.equ CMD_LINE, $03

; Colors
.equ COLOR_BLACK, $00
.equ COLOR_DIM, $01
.equ COLOR_MEDIUM, $02
.equ COLOR_WHITE, $03

; Keycodes
.equ KEY_UP, $01
.equ KEY_DOWN, $02
.equ KEY_LEFT, $03
.equ KEY_RIGHT, $04
```

#### 4.3 Create Shared VGC Library

**`lib/vgc_lib.asm`:**
```asm
.include "constants.asm"

; Clear screen to black
; Modifies: A
; Preserves: X, Y
clear_screen:
  LDA #CMD_CLEAR
  STA VGC_CMD
  LDA #COLOR_BLACK
  STA VGC_COLOR
  LDA #$01
  STA VGC_EXEC
  RTS

; Present frame (swap buffers)
; Modifies: A
; Preserves: X, Y
present_frame:
  LDA #$02
  STA VGC_CONTROL
  RTS

; Draw line from (X0,Y0) to (X1,Y1) with intensity in A
; Input: $10-$13 = X0,Y0,X1,Y1, A = intensity
; Modifies: A
; Preserves: X, Y
draw_line:
  PHA              ; Save intensity
  LDA #CMD_LINE
  STA VGC_CMD
  LDA $10          ; X0
  STA VGC_X0
  LDA $11          ; Y0
  STA VGC_Y0
  LDA $12          ; X1
  STA VGC_X1
  LDA $13          ; Y1
  STA VGC_Y1
  PLA              ; Restore intensity
  STA VGC_COLOR
  LDA #$01
  STA VGC_EXEC
  RTS
```

#### 4.4 Refactor Graphics Module

**`graphics.asm`:**
```asm
.include "lib/constants.asm"
.include "lib/vgc_lib.asm"

; Draw ship triangle at (SHIP_X, SHIP_Y)
; Modifies: A, $10-$13
; Preserves: X, Y
draw_ship:
  LDA #COLOR_WHITE
  PHA              ; Save color for all 3 lines

  ; Line 1: Top vertex to bottom-left
  LDA SHIP_X
  STA $10          ; X0
  LDA SHIP_Y
  SEC
  SBC #$06
  STA $11          ; Y0
  LDA SHIP_X
  SEC
  SBC #$04
  STA $12          ; X1
  LDA SHIP_Y
  CLC
  ADC #$04
  STA $13          ; Y1
  PLA
  PHA
  JSR draw_line

  ; Line 2: Bottom-left to bottom-right
  ; ... similar ...

  ; Line 3: Bottom-right to top vertex
  ; ... similar ...

  PLA              ; Clean up stack
  RTS

; Draw asteroid square at fixed position
draw_asteroid:
  ; ... implement ...
  RTS
```

#### 4.5 Write Tests

**`tests/graphics_test.asm`:**
```asm
.include "../lib/constants.asm"
.include "../graphics.asm"

.org $8000
test_ship:
  LDA #$80
  STA SHIP_X
  LDA #$90
  STA SHIP_Y

  JSR clear_screen
  JSR draw_ship
  JSR present_frame
  HLT
```

**`assembler/test/asteroids_graphics_test.cpp`:**
```cpp
TEST(AsteroidsRefactored, DrawShip) {
  auto fb = ExecuteAndRender(
      ReadFile("demos/asteroids/tests/graphics_test.asm"));

  // Verify ship rendered correctly
  ExpectLine(fb, 128, 138, 124, 148, 3);
  ExpectLine(fb, 124, 148, 132, 148, 3);
  ExpectLine(fb, 132, 148, 128, 138, 3);
}
```

**Deliverables:**
- [ ] Modular asteroids directory structure
- [ ] All constants extracted to constants.asm
- [ ] Shared VGC library (reusable across demos)
- [ ] Tests for graphics, input, physics modules
- [ ] Documentation showing module organization
- [ ] Verify asteroids still works correctly

---

## Success Criteria

- [ ] Can split 175-line file into 5-10 clear modules
- [ ] Zero magic numbers in code (all named constants)
- [ ] Automated tests verify graphics output matches expected
- [ ] Test failures show clear diagnostics (which pixel/line failed)
- [ ] New developer can understand code structure in <10 minutes
- [ ] Can add new feature by editing single module
- [ ] Build + test cycle under 5 seconds

## Testing Strategy

Each phase has dedicated tests:
- **Phase 1:** Lexer/parser unit tests, integration test with multi-file program
- **Phase 2:** Graphics command tests, subroutine rendering tests
- **Phase 3:** AssemblyTestRunner tests, example subroutine tests
- **Phase 4:** Tests for each asteroids module, regression test for full demo

All existing tests must continue passing.

## Documentation Updates

- [ ] `docs/assembly-guide.md` - Document `.include` and `.equ` syntax
- [ ] `docs/testing-guide.md` - Document graphics testing and assembly testing patterns
- [ ] `demos/README.md` - Document demo structure guidelines
- [ ] Update `assembler/README.md` with new directives

## Open Questions

1. **Asteroids hang issue:** Need to investigate what "hang at startup" means
   - Infinite loop is expected (no HLT in main program)
   - Does it fail to initialize? Graphics not showing?
   - Need to run with debugger or add trace output

2. **Macro implementation:** Start with manual patterns, add preprocessor later?
   - Phase 3 uses manual test patterns (no true macros yet)
   - Can add macro preprocessor in future phase if needed

3. **Frontend build:** Is SDL2 frontend built by default?
   - Need to check CMake options for frontend
   - May need headless testing approach for CI

## Timeline

- **Phase 1:** 3-4 days (foundation for everything else)
- **Phase 2:** 2 days (high impact, uses existing ImageBackend)
- **Phase 3:** 3-4 days (enables TDD workflow)
- **Phase 4:** 2-3 days (apply improvements to asteroids)
- **Total:** 10-13 days

## Dependencies

- ImageBackend exists and works (verified in exploration)
- Assembler pipeline is stable (5-stage design)
- GoogleTest infrastructure in place

## Risks

- Include mechanism may complicate error reporting (mitigation: careful span tracking)
- Graphics tests may be flaky due to pixel-perfect matching (mitigation: helper functions with clear error messages)
- Assembly testing may be verbose without macros (mitigation: start simple, add macros later if needed)
