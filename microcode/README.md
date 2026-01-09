# Microcode Module

YAML-defined microcode with compile-time code generation, multi-pass compilation, and encoding for CPU control.

## Overview

The microcode module defines instruction behavior as sequences of control signal assertions. Unlike a fluent C++ DSL, microcode is specified in a YAML file that directly represents the intermediate representation (IR). A Python script generates C++ code at build time, similar to the ISA module.

## Design Goals

- **YAML-first definition**: Microcode is data, not code. The YAML file is the source of truth.
- **Build-time code generation**: `microcode.yaml` → `generate_microcode.py` → C++ IR instantiation.
- **Singleton initialization**: HDL and compiled microcode are instantiated once per binary run.
- **Multi-pass compilation**: Transforms add fetch/decode, validators catch errors, optimizers merge steps.
- **No generality**: This system builds only the IRATA CPU. No pluggable architectures.

## System Overview

```
microcode.yaml → generate_microcode.py → microcode_ir.cpp
                                              ↓
                            C++ Compiler Pipeline (once at startup)
                                              ↓
                            FetchTransformer → SequenceTransformer
                                              ↓
                            Validators (Bus, Sequence, Fetch, Status)
                                              ↓
                            Optimizers (EmptyStep, DuplicateStep, StepMerger)
                                              ↓
                            Encoder → MicrocodeROM (singleton)
```

## YAML Microcode Format

### Path Syntax

Control paths use dot notation with implicit `/cpu` prefix. This syntax is shared between the YAML file and the HDL path resolution API.

| YAML Path | HDL Path |
|-----------|----------|
| `a.read` | `/cpu/a/read` |
| `memory.mar.low.write` | `/cpu/memory/mar/low/write` |
| `pc.increment` | `/cpu/pc/increment` |

Path resolution happens at YAML load time, not later in the pipeline. Invalid paths fail immediately with descriptive errors.

### Instruction Names

Instructions are referenced by their **canonical ISA name** (e.g., `LDA_IMM`, `BEQ`). The microcode system looks up opcodes from the ISA module—opcodes are never specified in the microcode YAML.

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

The IR is defined in C++ headers. The code generator produces instantiation code that populates these structures. Control paths are resolved to HDL pointers at generation time—the IR contains no strings.

```cpp
namespace irata2::microcode::ir {

// A single CPU tick's worth of control assertions
struct Step {
  int stage;
  std::vector<const hdl::ControlBase*> controls;  // Resolved at codegen time
};

// A single instruction variant (most have one with empty conditions)
struct Instruction {
  isa::Opcode opcode;  // Looked up from ISA by canonical name
  std::map<const hdl::Status*, bool> status_conditions;
  std::vector<Step> steps;
};

// The complete microcode definition
struct InstructionSet {
  std::vector<Instruction> instructions;
};

// Generated function that returns the IRATA microcode
const InstructionSet& GetIrataInstructionSet();

}  // namespace irata2::microcode::ir
```

### Path Resolution at Load Time

The code generator resolves all paths when parsing the YAML:

1. Parse YAML file
2. For each control path string (e.g., `"a.read"`):
   - Call `hdl::Cpu::ResolveControl("a.read")`
   - If resolution fails, abort with detailed error (see Error Reporting)
   - Store the resolved `const hdl::ControlBase*` in the IR
3. For each instruction name (e.g., `"LDA_IMM"`):
   - Call `isa::IsaInfo::GetInstruction("LDA_IMM")`
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

**BusValidator**: Ensures single-writer-per-bus-per-phase. Detects:
- Bus contention (multiple writers to same bus in one step)
- Missing drivers (readers without writers)

**SequenceValidator**: Verifies sequence counter discipline:
- Non-final steps must increment SC
- Final step must reset SC

**FetchValidator**: Ensures all instructions have identical stage 0 (the fetch/decode preamble).

**StatusValidator**: For conditional instructions, ensures complementary variants exist. If an instruction has `{Z: true}`, there must be a sibling with `{Z: false}`.

### Optimizers

Optimizers reduce ROM size while preserving correctness.

**EmptyStepOptimizer**: Removes steps with no controls (when safe).

**DuplicateStepOptimizer**: Collapses identical adjacent steps.

**StepMerger**: Merges adjacent steps within the same stage when tick phases permit. Two steps can merge if all controls in the first step are consumed (by tick phase) before or at the same time as controls in the second step begin.

```
Mergeable:     [a.write] + [b.read]     → [a.write, b.read]  (Write before Read)
Not mergeable: [a.read] + [b.write]     → kept separate      (Read after Write)
```

### Compilation Order

```
Raw IR from codegen
    ↓
FetchTransformer    (add stage 0)
    ↓
SequenceTransformer (add SC management)
    ↓
All Validators
    ↓
EmptyStepOptimizer
    ↓
All Validators
    ↓
DuplicateStepOptimizer
    ↓
All Validators
    ↓
StepMerger (runs to fixed point)
    ↓
All Validators
    ↓
Validated IR
```

## Encoder

The encoder converts validated IR to a lookup table indexed by `{opcode, step_number, status_flags}`.

### Control Encoding

Controls are sorted by path and assigned bit positions. A step's controls become an OR'd bitfield.

```cpp
class ControlEncoder {
  uint64_t Encode(const std::set<std::string>& control_paths) const;
  std::set<std::string> Decode(uint64_t value) const;
};
```

### Status Encoding

Status flags are bit-packed. Partial status conditions (don't-care bits) are expanded to all permutations.

```cpp
class StatusEncoder {
  uint8_t Encode(const std::map<std::string, bool>& statuses) const;
  // Expands {Z: true} to [{Z:true, N:false, C:false, V:false}, {Z:true, N:true, C:false, V:false}, ...]
  std::vector<uint8_t> ExpandPartial(const std::map<std::string, bool>& partial) const;
};
```

### ROM Structure

```cpp
struct MicrocodeKey {
  uint8_t opcode;
  uint8_t step;
  uint8_t status;
};

// Hash: (opcode << 16) | (step << 8) | status
using MicrocodeROM = std::unordered_map<uint32_t, uint64_t>;  // key → control_word
```

## Singleton Architecture

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

## Build Integration

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

## HDL Requirements

The HDL module must provide:

- **Status register** (`SR`) with individual `Status` wires for each flag (Z, N, C, V)
- **Controller components**: Instruction Register (`IR`), Sequence Counter (`SC`)
- **Path resolution API**: Dot-notation lookup matching YAML syntax

### HDL Path Resolution API

The HDL must expose a path resolution interface that uses the same syntax as the microcode YAML:

```cpp
namespace irata2::hdl {

class Cpu {
public:
  // Resolve dot-notation path to control pointer
  // Example: "a.read" → &a_.read_control_
  // Throws PathResolutionError if not found
  const ControlBase* ResolveControl(std::string_view path) const;

  // Resolve to status pointer
  // Example: "status.Z" → &status_.zero_
  const Status* ResolveStatus(std::string_view path) const;

  // List all valid control paths (for error messages)
  std::vector<std::string> AllControlPaths() const;

  // List all valid status paths
  std::vector<std::string> AllStatusPaths() const;
};

}  // namespace irata2::hdl
```

The resolution walks the component tree using the dot-separated segments. The implicit `/cpu` prefix means all paths are relative to the CPU root.

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

## Testing Strategy

- **Validator unit tests**: Each validator has tests for acceptance and rejection cases
- **Optimizer unit tests**: Verify merge behavior respects stage boundaries and phase rules
- **Encoder round-trip tests**: Encode → decode produces original control set
- **Integration tests**: Compile sample microcode, verify ROM output
- **Singleton tests**: Verify HDL and ROM are initialized exactly once

Because of the singleton pattern, tests share the same compiled microcode. Test isolation comes from creating fresh `sim::Cpu` instances, not from recompiling microcode.

## Module Boundaries

Clear separation of responsibilities:

| Module | Owns | Provides to Microcode |
|--------|------|----------------------|
| `base` | Byte, Word, TickPhase | Phase definitions for optimizer |
| `hdl` | CPU structure, controls, buses | Path resolution API, control pointers |
| `isa` | Instruction names, opcodes, addressing modes | Opcode lookup by canonical name |
| `microcode` | Control sequences, compilation, encoding | Compiled ROM for sim |

The microcode module **never** defines opcodes—it references ISA by name. The HDL module **never** knows about instruction semantics—it just provides structural metadata and path resolution.

## Dependencies

- `irata2::base` - Byte/Word types, TickPhase
- `irata2::hdl` - Control signal definitions, path resolution API
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
├── compiler/
│   ├── compiler.h           # Multi-pass orchestrator
│   ├── pass.h               # Base class for all passes
│   ├── fetch_transformer.h
│   ├── sequence_transformer.h
│   ├── bus_validator.h
│   ├── sequence_validator.h
│   ├── fetch_validator.h
│   ├── status_validator.h
│   ├── empty_step_optimizer.h
│   ├── duplicate_step_optimizer.h
│   └── step_merger.h
├── encoder/
│   ├── encoder.h            # ROM generation
│   ├── control_encoder.h
│   └── status_encoder.h
└── singleton.h              # GetHdlCpu(), GetMicrocodeROM()
```
