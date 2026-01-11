# Microcode Module

Microcode IR, compiler passes, and validation for CPU control. YAML codegen is implemented; ROM encoding remains planned.

See [docs/plan.md](../docs/plan.md) for the vertical-slice roadmap that ties microcode to sim, assembler, and tests.

## Design Note: Instruction Memory Encoding (Spark Moment)

The microcode compiler will output a sparse table keyed by `(opcode, step, status)` that maps to a set of asserted controls. The simulator will then **burn** this sparse table into a fully-populated instruction memory (ROM) that represents the controller's brain in a hardware-ish way.

Planned components:
- **PartialStatus**: Represents a partially specified set of status flags (don’t-care bits).
- **CompleteStatus**: Represents a fully specified set of status flags.
- **StatusEncoder**: Expands `PartialStatus` into all matching `CompleteStatus` permutations and encodes `CompleteStatus` into a binary address field.
- **ControlEncoder**: Assigns stable bit positions to controls and encodes/decodes control words (HDL traversal order).
- **InstructionEncoder**: Combines opcode, step, and encoded status into instruction-memory addressing.

The microcode pipeline produces the sparse table; the simulator’s controller will use these encoders to build the ROM image that drives control-line assertions each tick.

### Reference Example: pirata Burn-In Pattern

The `pirata` simulator implements the intended burn-in flow:

- `InstructionMemory.initialize(encoder)` generates a sparse microcode table.
- The table is **burned into ROM chips** when addressable in 16 bits; control words are split across multiple ROMs (one byte per ROM).
- If the address space exceeds 16-bit ROM addressing, it falls back to a sparse in-memory table.
- The controller reads `(opcode, step, statuses)` each tick and asserts the decoded controls.

This pattern is the model for IRATA2’s instruction memory and controller implementation.

## Overview

The microcode module defines instruction behavior as sequences of control signal assertions. The current MVP focuses on the IR, fast-fail control lookup, and a minimal compiler pipeline. YAML codegen and ROM encoding are the next steps.

## Design Goals

- **YAML-first definition**: Microcode is data, not code. The YAML file will be the source of truth.
- **Build-time code generation**: `microcode.yaml` → `generate_microcode.py` → C++ IR instantiation.
- **Singleton initialization**: HDL and compiled microcode are instantiated once per binary run (planned).
- **Multi-pass compilation**: Transforms add fetch/decode, validators catch errors, optimizers merge steps.
- **No generality**: This system builds only the IRATA CPU. No pluggable architectures.

## System Overview

```
microcode.yaml → generate_microcode.py → irata_instruction_set.cpp
                                              ↓
                            C++ Compiler Pipeline (once at startup)
                                              ↓
                            FetchTransformer → SequenceTransformer
                                              ↓
                            Validators (ISA Coverage, Sequence, Fetch)
                                              ↓
                            Optimizers (EmptyStep, DuplicateStep, StepMerger)  (planned)
                                              ↓
                            Encoder → MicrocodeROM (singleton)  (planned)
```

## YAML Microcode Format

### Path Syntax

Control paths use dot notation with no root prefix. This syntax is shared between the YAML file and the HDL path resolution API.

| YAML Path | HDL Path |
|-----------|----------|
| `a.read` | `a.read` |
| `memory.mar.low.write` | `memory.mar.low.write` |
| `pc.increment` | `pc.increment` |

Path resolution happens when the generated C++ IR is instantiated using the `ir::Builder`. Invalid paths fail immediately with descriptive errors.

### Instruction Names

Instructions are referenced by their **canonical ISA name** (e.g., `LDA_IMM`, `BEQ`). The microcode system looks up opcodes from the ISA module—opcodes are never specified in the microcode YAML. (This lookup is planned for the YAML generator.)

This maintains clear module boundaries:
- **ISA module**: Owns instruction definitions, opcodes, addressing modes
- **Microcode module**: Defines control sequences, references ISA by name

### Structure

```yaml
# microcode/microcode.yaml

# Fetch/decode preamble prepended to all instructions as stage 0
fetch_preamble:
  - [pc.write, memory.mar.write]
  - [memory.read, controller.ir.write]
  - [pc.increment]

instructions:
  HLT:
    stages:
      - steps:
          - [controller.halt]

  NOP:
    stages:
      - steps:
          - []  # empty step, will be optimized away

  LDA_IMM:
    stages:
      - steps:
          - [pc.write, memory.mar.low.read]
          - [memory.read, a.write]
          - [pc.increment]

  LDA_ZPG:
    stages:
      - steps:
          - [pc.write, memory.mar.low.read]
          - [memory.read, memory.mar.low.write]
          - [memory.read, a.write]
          - [pc.increment]

  # Conditional instruction with variants
  BEQ:
    variants:
      - when: { Z: true }   # branch taken
        stages:
          - steps:
              - [pc.write, memory.mar.low.read]
              - [memory.read, pc.low.write]
      - when: { Z: false }  # branch not taken
        stages:
          - steps:
              - [pc.increment]
```

### Default Variant

Most instructions have a single unconditional variant. These use a shorthand without the `variants` wrapper:

```yaml
LDA_IMM:
  stages:
    - steps:
        - [pc.write, memory.mar.low.read]
        # ...
```

This is equivalent to:

```yaml
LDA_IMM:
  variants:
    - when: {}  # empty = unconditional
      stages:
        - steps:
            - [pc.write, memory.mar.low.read]
```

### Stages vs Steps

- **Step**: A set of controls asserted during one CPU tick. Controls in a step execute in their designated tick phases (Control → Write → Read → Process → Clear).
- **Stage**: A sequence of steps that must stay together. The optimizer can merge steps *within* a stage but never *across* stage boundaries.

The primary use case for multiple stages is the fetch/decode preamble, which must be identical across all instructions. The `FetchTransformer` prepends stage 0 to all instructions, so user-defined stages start at stage 1.

## Intermediate Representation (IR)

The IR is defined in C++ headers. The code generator (planned) will produce instantiation code that populates these structures. Control paths are resolved to HDL pointers at instantiation time—the IR contains no strings for controls.

```cpp
namespace irata2::microcode::ir {

// A single CPU tick's worth of control assertions
struct Step {
  int stage;
  std::vector<const hdl::ControlInfo*> controls;  // Resolved at codegen time
};

// A single instruction variant (most have one with empty conditions)
struct InstructionVariant {
  std::map<std::string, bool> status_conditions;  // Status wiring planned
  std::vector<Step> steps;
};

// A single instruction with one or more variants
struct Instruction {
  isa::Opcode opcode;  // Looked up from ISA by canonical name
  std::vector<InstructionVariant> variants;
};

// The complete microcode definition
struct InstructionSet {
  std::vector<Step> fetch_preamble;
  std::vector<Instruction> instructions;
};

// Generated function that returns the IRATA microcode
const InstructionSet& GetIrataInstructionSet();

}  // namespace irata2::microcode::ir
```

## Compilation Output: MicrocodeProgram

The compiler emits a `MicrocodeProgram` that serves as the contract with the sim:

```cpp
namespace irata2::microcode::output {

struct MicrocodeKey {
  uint8_t opcode;
  uint8_t step;
  uint8_t status;
};

using MicrocodeTable = std::unordered_map<uint32_t, uint64_t>;

struct MicrocodeProgram {
  MicrocodeTable table;             // sparse (opcode, step, status) -> control word
  std::vector<std::string> control_paths;  // stable HDL traversal ordering
  std::vector<StatusBitDefinition> status_bits;  // name + bit for status encoding
};

}  // namespace irata2::microcode::output
```

The simulator consumes `MicrocodeProgram` to burn the sparse table into instruction memory.

### Path Resolution at Load Time

The code generator resolves all paths when parsing the YAML:

1. Parse YAML file (planned)
2. For each control path string (e.g., `"a.read"`):
   - Call `ir::CpuPathResolver::RequireControl("a.read")` via `ir::Builder`
   - If resolution fails, abort with detailed error (see Error Reporting)
   - Store the resolved `const hdl::ControlInfo*` in the IR
3. For each instruction name (e.g., `"LDA_IMM"`):
   - Call `isa::IsaInfo::GetInstruction("LDA_IMM")` (planned)
   - If not found, abort with error listing valid instruction names
   - Store the `isa::Opcode` in the IR

This "fail early, fail hard" approach catches typos and structural errors before any compilation passes run.

## Compiler Pipeline

The compiler runs once at program startup, transforming the raw IR into an optimized, validated ROM.

### Transformers

**FetchTransformer**: Prepends the `fetch_preamble` from the YAML file to all instructions as stage 0. User-defined stages are renumbered starting at 1. The preamble is defined once in YAML and shared across all instructions, ensuring uniformity.

**SequenceTransformer**: Ensures proper sequence counter management:
- All non-final steps: assert `controller.sc.increment`
- Final step: assert `controller.sc.reset`

### Validators

Validators run after each transform and optimization pass. Any failure aborts compilation with a descriptive error.

**IsaCoverageValidator (MVP)**: Ensures microcode defines all ISA instructions exactly once.

**SequenceValidator (MVP)**: Verifies sequence counter discipline:
- Non-final steps must increment SC
- Final step must reset SC

**FetchValidator (MVP)**: Ensures all instructions have identical stage 0 (the fetch/decode preamble).

**BusValidator**: Ensures single-writer-per-bus-per-phase. Detects:
- Bus contention (multiple writers to same bus in one step)
- Missing drivers (readers without writers)

**StatusValidator**: For conditional instructions, ensures complementary variants exist. If an instruction has `{Z: true}`, there must be a sibling with `{Z: false}`.

### Optimizers

Optimizers reduce ROM size while preserving correctness. (Planned.)

**EmptyStepOptimizer**: Removes steps with no controls (when safe).

**DuplicateStepOptimizer**: Collapses identical adjacent steps.

**StepMerger**: Merges adjacent steps within the same stage when tick phases permit. Two steps can merge if all controls in the first step are consumed (by tick phase) before or at the same time as controls in the second step begin.

```
Mergeable:     [a.write] + [b.read]     → [a.write, b.read]  (Write before Read)
Not mergeable: [a.read] + [b.write]     → kept separate      (Read after Write)
```

### Compilation Order (MVP)

```
Raw IR from codegen
    ↓
FetchTransformer    (add stage 0)
    ↓
SequenceTransformer (add SC management)
    ↓
FetchValidator
IsaCoverageValidator
SequenceValidator
    ↓
Validated IR
```

## Encoder

The encoder converts validated IR to a lookup table indexed by `{opcode, step_number, status_flags}`. (Planned.)

### Control Encoding (Planned)

Controls are sorted by path and assigned bit positions. A step's controls become an OR'd bitfield.

```cpp
class ControlEncoder {
  uint64_t Encode(const std::set<std::string>& control_paths) const;
  std::set<std::string> Decode(uint64_t value) const;
};
```

### Status Encoding (Planned)

Status flags are bit-packed. Partial status conditions (don't-care bits) are expanded to all permutations.

```cpp
class StatusEncoder {
  uint8_t Encode(const std::map<std::string, bool>& statuses) const;
  // Expands {Z: true} to [{Z:true, N:false, C:false, V:false}, {Z:true, N:true, C:false, V:false}, ...]
  std::vector<uint8_t> ExpandPartial(const std::map<std::string, bool>& partial) const;
};
```

### ROM Structure (Planned)

```cpp
struct MicrocodeKey {
  uint8_t opcode;
  uint8_t step;
  uint8_t status;
};

// Hash: (opcode << 16) | (step << 8) | status
using MicrocodeROM = std::unordered_map<uint32_t, uint64_t>;  // key → control_word
```

## Singleton Architecture (Planned)

HDL and microcode are compiled once per binary run. This eliminates the performance overhead of repeated initialization in tests.

```cpp
namespace irata2 {

// Singleton access to immutable structures
const hdl::Cpu& GetHdlCpu();
const microcode::MicrocodeROM& GetMicrocodeROM();

// sim::Cpu instances reference the singletons
class sim::Cpu {
public:
  Cpu();  // Uses GetHdlCpu() and GetMicrocodeROM()
  // Mutable runtime state...
};

}  // namespace irata2
```

Multiple `sim::Cpu` instances share the same immutable HDL and ROM. Each sim instance has its own mutable state (register values, bus contents, etc.).

## Build Integration (Planned)

Similar to the ISA module:

```cmake
# microcode/CMakeLists.txt
add_custom_command(
  OUTPUT ${MICROCODE_CPP}
  COMMAND ${Python3_EXECUTABLE} ${MICROCODE_GENERATOR} ${MICROCODE_YAML} ${MICROCODE_CPP}
  DEPENDS ${MICROCODE_YAML} ${MICROCODE_GENERATOR}
  COMMENT "Generating microcode IR from YAML"
)
```

## Future: Macro System

For complex controls like ALU operations, a macro system will allow shorthands:

```yaml
# Future syntax
macros:
  alu.add: [alu.opcode_0, alu.opcode_2]  # binary 0101 = ADD
  alu.sub: [alu.opcode_1, alu.opcode_2]  # binary 0110 = SUB

instructions:
  ADC_IMM:
    opcode: 0x10
    stages:
      - steps:
          - [a.write, alu.lhs.read]
          - [pc.write, memory.mar.low.read]
          - [memory.read, alu.rhs.read, alu.add]  # macro expands here
          - [alu.result.write, a.read]
```

## HDL Requirements (MVP implemented where noted)

The HDL module must provide:

- **Status register** (`SR`) with 6502-style flags (N, V, U, B, D, I, Z, C), bus-connected for push/pop (planned)
- **Controller components**: Instruction Register (`IR`), Sequence Counter (`SC`)
- **Path resolution support**: Controls expose stable `ControlInfo` with dot paths

### Microcode Path Resolution API (MVP implemented)

Path resolution lives in the microcode module and uses the HDL visitor to build
an index of `ControlInfo` pointers:

```cpp
namespace irata2::microcode::ir {

class CpuPathResolver {
public:
  explicit CpuPathResolver(const hdl::Cpu& cpu);

  // Resolve dot-notation path to control info
  // Example: "a.read" -> &a_.read_control_.control_info()
  const hdl::ControlInfo* RequireControl(std::string_view path,
                                         std::string_view context) const;

  // List all valid control paths (for error messages)
  const std::vector<std::string>& AllControlPaths() const;
};

}  // namespace irata2::microcode::ir
```

The resolver indexes controls during construction and is used by `ir::Builder`.

## Error Reporting

Microcode development is the core of CPU implementation work. Errors must be verbose, informative, and actionable.

### Error Message Requirements

All errors include:
- **Location**: File, line (if applicable), instruction name, variant index, stage index, step index
- **Context**: The full step or instruction being processed
- **Problem**: Clear description of what's wrong
- **Suggestions**: Valid alternatives, similar paths, or fix guidance

### Example Error Messages

**Path resolution failure:**
```
microcode.yaml: Error in instruction LDA_IMM, stage 0, step 1
  Control path 'a.raed' not found

  Step: [pc.write, a.raed]
                   ^^^^^^

  Did you mean 'a.read'?

  Valid controls starting with 'a.':
    a.read
    a.write
```

**Bus contention:**
```
microcode.yaml: Bus contention in instruction STA_ABS, stage 1, step 2
  Multiple writers to 'data_bus' in same step

  Step: [a.write, memory.write]
         ^^^^^^^  ^^^^^^^^^^^^
         writes   also writes
         to       to data_bus
         data_bus

  Controls cannot both write to the same bus in one tick.
  Split into separate steps:
    - [a.write]
    - [memory.write]
```

**Unknown instruction:**
```
microcode.yaml: Unknown instruction 'LDA_IMMEDIATE'

  Did you mean 'LDA_IMM'?

  Valid instruction names:
    LDA_IMM, LDA_ZPG, LDA_ZPX, LDA_ABS, LDA_ABX, LDA_ABY
    LDX_IMM, LDX_ZPG, LDX_ABS
    ...
```

**Missing status complement:**
```
microcode.yaml: Incomplete status variants for instruction BEQ

  Found variant with { Z: true } but no complement with { Z: false }

  Conditional instructions must have variants for all status combinations.
  Add a variant:
    - when: { Z: false }
      stages:
        - steps:
            - [pc.increment]  # skip branch
```

## Testing Strategy (MVP implemented for builder, transformers, validators)

- **Validator unit tests**: Each validator has tests for acceptance and rejection cases
- **Transformer unit tests**: Fetch/sequence transformers cover stage/preamble behavior
- **Optimizer unit tests**: Verify merge behavior respects stage boundaries and phase rules (planned)
- **Encoder round-trip tests**: Encode → decode produces original control set (planned)
- **Integration tests**: Compile sample microcode, verify ROM output (planned)
- **Singleton tests**: Verify HDL and ROM are initialized exactly once (planned)

Because of the singleton pattern, tests share the same compiled microcode. Test isolation comes from creating fresh `sim::Cpu` instances, not from recompiling microcode.

## Module Boundaries

Clear separation of responsibilities:

| Module | Owns | Provides to Microcode |
|--------|------|----------------------|
| `base` | Byte, Word, TickPhase | Phase definitions for optimizer |
| `hdl` | CPU structure, controls, buses | ControlInfo metadata and stable paths |
| `isa` | Instruction names, opcodes, addressing modes | Opcode lookup by canonical name |
| `microcode` | Control sequences, compilation, encoding | Compiled ROM for sim |

The microcode module **never** defines opcodes—it references ISA by name. The HDL module **never** knows about instruction semantics—it just provides structural metadata and stable control metadata.

## Dependencies

- `irata2::base` - Byte/Word types, TickPhase
- `irata2::hdl` - Control signal definitions and stable control metadata
- `irata2::isa` - Instruction definitions, opcode lookup by name

## Files

```
microcode/
├── microcode.yaml           # Source of truth for instruction microcode
├── generate_microcode.py    # YAML → C++ code generator
├── ir/
│   ├── instruction_set.h    # IR data structures
│   ├── step.h
│   └── instruction.h
│   └── builder.h            # Fast-fail control lookup helper
│   └── cpu_path_resolver.h  # Path-to-control index for DSL parsing
├── compiler/
│   ├── compiler.h           # Multi-pass orchestrator
│   ├── pass.h               # Base class for all passes
│   ├── fetch_transformer.h
│   ├── sequence_transformer.h
│   ├── fetch_validator.h
│   ├── sequence_validator.h
│   └── isa_coverage_validator.h
├── encoder/
│   ├── encoder.h            # ROM generation
│   ├── control_encoder.h
│   └── status_encoder.h
└── output/                  # Compiled ROM output types

## Current Status

Implemented:
- IR data structures, including fetch preamble and instruction variants
- `ir::Builder` that resolves control paths via `ir::CpuPathResolver` with fast-fail errors
- MVP compiler passes: `FetchTransformer`, `SequenceTransformer`
- MVP validators: `FetchValidator`, `SequenceValidator`, `IsaCoverageValidator`
- Encoders for instruction keys, control bits, and status bits
- `output::MicrocodeProgram` table for compiled control words
- Unit tests covering builder, compiler, and encoders

Next Steps:
1. Add status-aware variants and status condition validation in the IR/compiler.
2. Implement bus/status validators and optimizer passes.
3. Emit ROM images for hardware-style instruction memories.
```
