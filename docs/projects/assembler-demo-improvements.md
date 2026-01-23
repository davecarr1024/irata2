# Assembler and Demo Development Improvements

## Overview

This document outlines improvements to the assembler and tooling to make demo programs easier to develop, test, and debug. The goal is to enable modular assembly development with proper testing infrastructure.

## Current State

### Assembler Capabilities
- ✓ 153 IRATA2 ISA instructions with 12 addressing modes
- ✓ Global label support (forward and backward references)
- ✓ Two directives: `.org` (set address) and `.byte` (inline data)
- ✓ Debug JSON output with symbol table and PC→source mapping
- ✓ Source span tracking for error messages
- ✗ **No file inclusion mechanism**
- ✗ **No support for named constants/equates**
- ✗ **No local labels or scoping**
- ✗ **No macros or code generation**

### Demo Development
- ✓ Working demos: blink.asm, move_sprite.asm, asteroids.asm
- ✓ VGC ImageBackend for headless rendering tests
- ✓ DemoRunner with SDL2 for interactive execution
- ✓ Integration test infrastructure (assembler/test/integration/)
- ✗ **All code must be in single file** (no modularity)
- ✗ **Magic numbers for zero-page variables** ($00, $01, etc.)
- ✗ **No subroutine-level testing**
- ✗ **Limited graphics verification** (manual inspection only)

### Testing Gaps
- Demo programs only tested for successful compilation
- No automated verification of program behavior
- Graphics output not tested (despite ImageBackend availability!)
- No way to test individual subroutines in isolation

---

## Proposed Improvements

### Phase 1: File Inclusion and Named Symbols (Essential Foundation)

**Priority: CRITICAL - Enables all other improvements**

#### 1.1 Add `.include` Directive

**Syntax:**
```asm
.include "path/to/file.asm"
```

**Behavior:**
- Inline file contents at include location during parsing
- Path relative to including file's directory
- Recursive includes allowed (with cycle detection)
- All labels from included files enter global namespace
- Source spans track original file locations

**Implementation:**
```cpp
// In DirectiveStmt::Type enum
enum class Type { Org, Byte, Include };

// In Parser
void Parser::ParseIncludeDirective() {
  std::string path = ExpectString();  // New string literal token type
  std::string full_path = ResolveIncludePath(current_file_dir_, path);

  // Cycle detection
  if (include_stack_.contains(full_path)) {
    throw AssemblerError("Circular include detected", ...);
  }

  // Recursively parse included file
  include_stack_.insert(full_path);
  std::string included_source = ReadFile(full_path);
  Lexer sub_lexer(included_source, full_path);
  Parser sub_parser(sub_lexer.Lex(), current_file_dir_);
  Program sub_program = sub_parser.Parse();

  // Merge into current program's statement list
  program_.statements.insert(program_.statements.end(),
                             sub_program.statements.begin(),
                             sub_program.statements.end());

  include_stack_.erase(full_path);
}
```

**Tests:**
- Include single file
- Include multiple files
- Nested includes (A includes B, B includes C)
- Circular include detection (A includes B, B includes A)
- Label resolution across files
- Error spans show correct file locations

**Use Case:**
```asm
; main.asm
.include "zero_page.asm"    ; Variable definitions
.include "vgc.asm"          ; Graphics subroutines
.include "input.asm"        ; Input handling

.org $8000
  JSR init_graphics
  JSR main_loop
```

#### 1.2 Add `.equ` Directive (Named Constants)

**Syntax:**
```asm
.equ NAME, value
.equ SHIP_X, $00        ; Zero page address
.equ SHIP_Y, $01
.equ VGC_CMD, $4100     ; MMIO register
.equ COLOR_WHITE, $03   ; Intensity value
```

**Behavior:**
- Define named constant (not an address label!)
- Can be used in any numeric context
- Evaluated at parse time (before first pass)
- Support arithmetic expressions: `.equ SCREEN_SIZE, 256`

**Implementation:**
```cpp
// New symbol table for constants (separate from address labels)
std::unordered_map<std::string, uint32_t> constants_;

// In operand resolution
uint32_t ResolveConstant(const std::string& name) {
  if (constants_.contains(name)) {
    return constants_.at(name);
  }
  throw AssemblerError("Unknown constant: " + name, ...);
}
```

**Tests:**
- Define and use constants
- Constants in all addressing modes
- Forward references to constants
- Constant arithmetic (if expressions supported)
- Error on duplicate constant names

**Use Case:**
```asm
; zero_page.asm
.equ SHIP_X, $00
.equ SHIP_Y, $01
.equ ASTEROID_X, $02
.equ ASTEROID_Y, $03

; asteroids.asm
.include "zero_page.asm"

init:
  LDA #$80
  STA SHIP_X        ; Instead of STA $00
  LDA #$90
  STA SHIP_Y        ; Instead of STA $01
```

#### 1.3 Add String Literal Tokens

**Required for `.include` directive**

**Lexer changes:**
```cpp
// Token.h
enum class TokenKind {
  // ... existing ...
  String,  // "path/to/file.asm"
};

struct Token {
  std::optional<std::string> string_value;  // New field
};

// In Lexer::Lex()
if (current_ == '"') {
  return LexStringLiteral();
}
```

**String escaping:**
- Support `\"` for embedded quotes
- Support `\\` for backslashes
- No other escape sequences needed initially

---

### Phase 2: Graphics Testing Framework (High Impact)

**Priority: HIGH - Leverages existing ImageBackend**

#### 2.1 Add VGC Framebuffer Assertion Helpers

**Create `assembler/test/graphics_test_helpers.h`:**

```cpp
namespace irata2::assembler::test {

// Execute assembly program and return framebuffer
ImageBackend ExecuteAndRender(const std::string& source,
                               size_t max_cycles = 10000);

// Assertion helpers
void ExpectPixel(const ImageBackend& backend,
                 uint8_t x, uint8_t y, uint8_t intensity);

void ExpectLine(const ImageBackend& backend,
                uint8_t x0, uint8_t y0,
                uint8_t x1, uint8_t y1,
                uint8_t intensity);

void ExpectClear(const ImageBackend& backend, uint8_t intensity);

// Count pixels with given intensity
size_t CountPixels(const ImageBackend& backend, uint8_t intensity);

// Save framebuffer as text for debugging
std::string DumpFramebuffer(const ImageBackend& backend);

}  // namespace
```

**Implementation:**
```cpp
ImageBackend ExecuteAndRender(const std::string& source, size_t max_cycles) {
  // Assemble
  Cartridge cart = Assemble(source, Options{});

  // Create CPU with ImageBackend
  auto backend = std::make_unique<ImageBackend>();
  ImageBackend* backend_ptr = backend.get();

  auto region_factory = std::make_unique<Memory::RegionFactory>();
  region_factory->AddRom(0x8000, std::move(cart.rom));
  region_factory->AddVgc(0x4100, std::move(backend));

  Cpu cpu(std::move(region_factory));

  // Run until halt or max cycles
  cpu.Reset();
  for (size_t i = 0; i < max_cycles; ++i) {
    cpu.Tick();
    if (cpu.GetHaltReason() != HaltReason::None) break;
  }

  return *backend_ptr;
}
```

**Tests:**
```cpp
TEST(GraphicsHelperTest, DrawSinglePoint) {
  const char* source = R"(
    .org $8000
    LDA #$02        ; POINT command
    STA $4100
    LDA #$80        ; X=128
    STA $4101
    LDA #$90        ; Y=144
    STA $4102
    LDA #$03        ; White
    STA $4105
    LDA #$01        ; Execute
    STA $4106
    HLT
  )";

  ImageBackend fb = ExecuteAndRender(source);
  ExpectPixel(fb, 128, 144, 3);  // White pixel at center
  EXPECT_EQ(CountPixels(fb, 3), 1);  // Only one pixel drawn
}
```

#### 2.2 Add Graphics Subroutine Tests

**Pattern for testing rendering subroutines:**

```asm
; tests/graphics/draw_triangle.asm
; Test: Draw triangle subroutine
; Expected: 3 lines forming triangle, vertices at (10,10), (20,10), (15,20)

.include "graphics_lib.asm"

.org $8000
  ; Set triangle vertices in zero page
  LDA #$0A
  STA triangle_x0    ; 10
  STA triangle_y0    ; 10
  LDA #$14
  STA triangle_x1    ; 20
  STA triangle_y1    ; 10
  LDA #$0F
  STA triangle_x2    ; 15
  LDA #$14
  STA triangle_y2    ; 20

  ; Call subroutine
  JSR draw_triangle

  ; Present and halt
  LDA #$02
  STA $4107
  HLT
```

**Corresponding C++ test:**
```cpp
TEST(GraphicsSubroutineTest, DrawTriangle) {
  std::string source = ReadFile("tests/graphics/draw_triangle.asm");
  ImageBackend fb = ExecuteAndRender(source);

  // Verify 3 lines
  ExpectLine(fb, 10, 10, 20, 10, 3);  // Base
  ExpectLine(fb, 20, 10, 15, 20, 3);  // Right edge
  ExpectLine(fb, 15, 20, 10, 10, 3);  // Left edge
}
```

---

### Phase 3: Assembly Testing Infrastructure

**Priority: MEDIUM - Enables TDD for assembly**

#### 3.1 Add `.test` Directive for Assertions

**Syntax:**
```asm
.test REGISTER, expected_value, "Description"

; Examples:
.test A, $42, "Accumulator should be 0x42"
.test X, $00, "X register should be zero"
.test $00, $80, "Ship X position should be 128"
```

**Behavior:**
- Only active when assembled with `--enable-tests` flag
- Reads register/memory value and compares to expected
- On mismatch: halt CPU and set special test failure code
- Ignored in normal builds (no-op or removed)

**Implementation:**
```cpp
// In simulator (not assembler!)
// Assembler just emits special instruction or data marker

// Could use undefined opcode or special MMIO device at $4200
#define TEST_DEVICE_BASE 0x4200
  // $4200: TEST_REGISTER (0=A, 1=X, 2=Y, 3=memory)
  // $4201: TEST_ADDR_LO (if memory test)
  // $4202: TEST_ADDR_HI
  // $4203: TEST_EXPECTED
  // $4204: TEST_EXECUTE (write 1 to run test, halts on failure)
```

**Alternative: Macro-based Approach (No simulator changes)**
```asm
; Generate this from .test directive:
.test A, $42, "Test description"

; Expands to:
  PHA               ; Save A
  CMP #$42
  BEQ test_pass_1
  CRS               ; Crash with test failure
test_pass_1:
  PLA               ; Restore A
```

#### 3.2 Add Test Runner Infrastructure

**File: `assembler/test/assembly_test_runner.h`**

```cpp
struct AssemblyTestResult {
  bool passed;
  std::string description;
  std::optional<std::string> failure_message;
  size_t cycles_executed;
};

class AssemblyTestRunner {
 public:
  // Run .asm test file, return results for all .test directives
  std::vector<AssemblyTestResult> RunTestFile(const std::string& path);

  // Run and expect pass
  void ExpectTestPass(const std::string& source);
};
```

**Integration with GoogleTest:**
```cpp
TEST(AsteroidSubroutines, UpdateShipPosition) {
  AssemblyTestRunner runner;
  runner.ExpectTestPass("tests/asteroids/update_ship.asm");
}
```

---

### Phase 4: Advanced Assembly Features

**Priority: MEDIUM-LOW - Quality of life improvements**

#### 4.1 Local Labels

**Syntax:**
```asm
global_label:
  LDA #$00
.local_loop:          ; Scoped to global_label
  INX
  BNE .local_loop
  RTS

another_global:
.local_loop:          ; Different scope, no conflict!
  DEX
  BNE .local_loop
  RTS
```

**Scoping rules:**
- Local labels start with `.`
- Scoped to previous global label
- Mangled to `global_label.local_loop` internally

#### 4.2 Anonymous Labels

**Syntax:**
```asm
  LDA #$00
:             ; Anonymous label
  INX
  BNE :-      ; Branch to previous :
  CPX #$10
  BNE :+      ; Branch to next :
:             ; Another anonymous label
  RTS
```

**Implementation:**
- `:` creates numbered label
- `:-` references previous
- `:+` references next

#### 4.3 Arithmetic Expressions

**Syntax:**
```asm
.equ SCREEN_WIDTH, 256
.equ SCREEN_HEIGHT, 256
.equ SCREEN_CENTER_X, SCREEN_WIDTH / 2
.equ SCREEN_CENTER_Y, SCREEN_HEIGHT / 2

LDA #(SCREEN_WIDTH - 10)
STA buffer + 5
```

**Operators:**
- `+`, `-`, `*`, `/`, `%` (modulo)
- `&`, `|`, `^` (bitwise)
- `<<`, `>>` (shifts)
- `()` for grouping

#### 4.4 Macros

**Syntax:**
```asm
.macro PUSH_REGS
  PHA
  TXA
  PHA
  TYA
  PHA
.endmacro

.macro POP_REGS
  PLA
  TAY
  PLA
  TAX
  PLA
.endmacro

; Usage:
my_function:
  PUSH_REGS
  ; ... function body ...
  POP_REGS
  RTS
```

**Features:**
- Text substitution (no parameters initially)
- Parameters later: `.macro SET_PIXEL, x, y, color`

---

## Implementation Plan

### Milestone 1: File Inclusion (2-3 days)

**Deliverables:**
- [ ] Add string literal tokens to lexer
- [ ] Implement `.include` directive in parser
- [ ] Add include path resolution
- [ ] Implement circular include detection
- [ ] Update error spans to track source files
- [ ] Add tests for include scenarios
- [ ] Update documentation

**Validation:**
- Split asteroids.asm into modular files
- Verify compilation and execution
- Check error messages reference correct files

### Milestone 2: Named Constants (1 day)

**Deliverables:**
- [ ] Implement `.equ` directive
- [ ] Add constants symbol table
- [ ] Update operand resolution
- [ ] Add tests for constant usage
- [ ] Document `.equ` syntax

**Validation:**
- Refactor asteroids.asm to use named constants
- Verify all magic numbers eliminated
- Check assembly produces identical binary

### Milestone 3: Graphics Testing (2 days)

**Deliverables:**
- [ ] Create `graphics_test_helpers.h/cpp`
- [ ] Implement `ExecuteAndRender()`
- [ ] Add pixel/line/clear assertion helpers
- [ ] Write example graphics tests
- [ ] Document graphics testing patterns

**Validation:**
- Write tests for all VGC commands (CLEAR, POINT, LINE)
- Test blink.asm rendering output
- Verify ImageBackend integration

### Milestone 4: Assembly Test Infrastructure (3-4 days)

**Deliverables:**
- [ ] Design `.test` directive or macro approach
- [ ] Implement test execution in simulator or as macros
- [ ] Create `AssemblyTestRunner` class
- [ ] Add GoogleTest integration
- [ ] Write example subroutine tests
- [ ] Document assembly testing patterns

**Validation:**
- Write subroutine-level tests for asteroids modules
- Verify test failures are caught
- Check test output is readable

### Milestone 5: Advanced Features (Future)

**Deliverables:**
- [ ] Local labels (`.label` syntax)
- [ ] Anonymous labels (`:`, `:-`, `:+`)
- [ ] Arithmetic expressions
- [ ] Basic macros

---

## Refactored Demo Structure Example

### Before (Current asteroids.asm)
```
asteroids.asm (175 lines, monolithic)
├─ Variable definitions (magic numbers)
├─ Input handling (inlined)
├─ Graphics drawing (inlined)
└─ Main loop
```

### After (Modular Structure)
```
asteroids/
├── main.asm                  ; Entry point and main loop
├── constants.asm             ; .equ definitions for all addresses
├── zero_page.asm             ; Variable layout documentation
├── input.asm                 ; Input polling subroutines
│   └── tests/
│       └── input_test.asm    ; Test input handling
├── graphics.asm              ; VGC drawing subroutines
│   └── tests/
│       ├── draw_ship_test.asm
│       └── draw_asteroid_test.asm
├── physics.asm               ; Movement and collision
│   └── tests/
│       └── update_position_test.asm
└── game_state.asm            ; Game state management
```

**Example: `constants.asm`**
```asm
; Zero page variables
.equ SHIP_X, $00
.equ SHIP_Y, $01
.equ ASTEROID_X, $02
.equ ASTEROID_Y, $03

; MMIO addresses
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

**Example: `graphics.asm`**
```asm
.include "constants.asm"

; Draw ship at (SHIP_X, SHIP_Y)
; Modifies: A
; Preserves: X, Y
draw_ship:
  ; Draw triangle with 3 lines
  LDA #CMD_LINE
  STA VGC_CMD

  ; Line 1: Top vertex to bottom-left
  LDA SHIP_X
  STA VGC_X0
  LDA SHIP_Y
  SEC
  SBC #$06        ; Y - 6
  STA VGC_Y0

  ; ... (more drawing code)

  RTS

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
```

**Example: `graphics_test.asm`**
```asm
.include "constants.asm"
.include "graphics.asm"

.org $8000

test_draw_ship:
  ; Set ship position
  LDA #$80
  STA SHIP_X      ; X = 128
  LDA #$90
  STA SHIP_Y      ; Y = 144

  ; Draw ship
  JSR clear_screen
  JSR draw_ship
  JSR present_frame

  HLT
```

**Corresponding C++ test:**
```cpp
TEST(AsteroidsGraphics, DrawShipCentered) {
  std::string source = ReadFile("demos/asteroids/tests/graphics_test.asm");
  ImageBackend fb = ExecuteAndRender(source, 10000);

  // Verify ship triangle is drawn at (128, 144)
  // Triangle vertices: (128, 138), (124, 148), (132, 148)
  ExpectLine(fb, 128, 138, 124, 148, COLOR_WHITE);  // Left edge
  ExpectLine(fb, 124, 148, 132, 148, COLOR_WHITE);  // Base
  ExpectLine(fb, 132, 148, 128, 138, COLOR_WHITE);  // Right edge
}
```

---

## Additional Tooling Improvements

### 1. Assembler CLI Enhancements

**Add `--watch` mode:**
```bash
irata2_asm --watch demos/asteroids/main.asm output.bin output.json
# Auto-recompile on file changes (for rapid iteration)
```

**Add `--print-symbols` flag:**
```bash
irata2_asm --print-symbols main.asm
# Output:
# SHIP_X         = 0x0000
# SHIP_Y         = 0x0001
# main_loop      = 0x8000
# draw_ship      = 0x8023
```

**Add `--disassemble` flag:**
```bash
irata2_asm --disassemble output.bin
# Disassemble compiled binary back to assembly (with labels from debug JSON)
```

### 2. Makefile Integration

**Add demo build targets:**
```makefile
.PHONY: demos
demos:
	@mkdir -p build/demos
	./build/assembler/irata2_asm demos/asteroids/main.asm \
		build/demos/asteroids.bin build/demos/asteroids.json

.PHONY: run-asteroids
run-asteroids: demos
	./build/frontend/irata2_demo \
		--rom build/demos/asteroids.bin \
		--fps 30 --scale 3 --cycles-per-frame 3333
```

### 3. Debugging Improvements

**Add breakpoint support in DemoRunner:**
```bash
irata2_demo --rom asteroids.bin --breakpoint 0x8023 --breakpoint draw_ship
# Pause execution at address or label, drop to debugger
```

**Add trace output:**
```bash
irata2_demo --rom asteroids.bin --trace-file trace.txt --trace-range 0x8000-0x8100
# Log all executed instructions in address range
```

### 4. Documentation

**Create `docs/assembly-guide.md`:**
- Assembly syntax reference
- Directive documentation
- Testing patterns
- Module organization best practices
- Graphics programming examples

**Create `demos/README.md`:**
- How to build and run demos
- Demo structure guidelines
- Testing demo programs
- Debugging tips

---

## Success Criteria

### For Assembler Improvements:
- [ ] Can split 175-line asteroids.asm into 5-10 modular files
- [ ] All magic numbers replaced with named constants
- [ ] Each module has clear interface (documented subroutines)
- [ ] Error messages show correct file:line:column for includes

### For Testing Infrastructure:
- [ ] Can write assembly test for graphics subroutine
- [ ] Test verifies rendered output matches expected pixels
- [ ] Test runs in <100ms (headless, no SDL)
- [ ] Test failures show clear diagnostics

### For Demo Development:
- [ ] New developer can understand asteroids code structure in <10 minutes
- [ ] Can add new asteroid type by editing single module
- [ ] Can test collision detection without running full game
- [ ] Can verify graphics changes with automated tests

### For Overall Developer Experience:
- [ ] Build + test cycle under 5 seconds
- [ ] Clear error messages for common mistakes
- [ ] Comprehensive examples and documentation
- [ ] Debugging tools help diagnose issues quickly

---

## Timeline Estimate

| Phase | Estimated Effort | Priority |
|-------|------------------|----------|
| File Inclusion | 2-3 days | **CRITICAL** |
| Named Constants | 1 day | **HIGH** |
| Graphics Testing | 2 days | **HIGH** |
| Assembly Tests | 3-4 days | **MEDIUM** |
| Advanced Features | 5-7 days | **LOW** |
| Documentation | 2 days | **MEDIUM** |
| **Total** | **15-23 days** | - |

**Recommended Order:**
1. File Inclusion + Named Constants (foundation)
2. Graphics Testing (high impact, uses existing ImageBackend)
3. Refactor asteroids.asm to modular structure (validate improvements)
4. Assembly Test Infrastructure (enables TDD)
5. Advanced Features (quality of life)
6. Documentation (throughout)

---

## Open Questions

1. **Asteroids crash**: What is the specific crash you're seeing?
   - Simulator halts with CRS instruction?
   - Segfault in C++ code?
   - Infinite loop timeout?
   - Graphics glitch?

2. **Build system**: Should assembler auto-assemble `.asm` files in `demos/`?
   - CMake custom targets for each demo?
   - Or manual `make demos` target?

3. **Test organization**: Where should graphics tests live?
   - `assembler/test/graphics/` (with C++ test harness)?
   - `demos/*/tests/` (alongside demo code)?
   - Both (assembly tests checked by C++ runner)?

4. **Macro complexity**: How powerful should macros be?
   - Simple text substitution only?
   - Parameters with type checking?
   - Conditional assembly (`.ifdef`, `.ifndef`)?

5. **ISA extensions for testing**: Should we add test-specific instructions?
   - Dedicated test assertion opcode?
   - Or use MMIO device approach?
   - Or pure software (macro-based tests)?

---

## Next Steps

Please review this plan and let me know:
1. What is the specific crash you're experiencing with asteroids?
2. Which improvements are highest priority for your current work?
3. Should I start with Phase 1 (File Inclusion) implementation?
4. Any changes or additions to the proposed design?

I'm ready to begin implementation as soon as you approve the approach!
