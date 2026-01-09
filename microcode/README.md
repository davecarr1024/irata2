# Microcode Module

Domain-specific language (DSL), intermediate representation (IR), compiler, and encoder for CPU microcode.

## Overview

The microcode module translates high-level instruction implementations into control signal sequences that drive the CPU through each instruction's execution phases.

## Design Doc

### Goals

- Encode instruction behavior as control signal sequences, not in sim components.
- Keep HDL and microcode strongly typed and immutable.
- Provide compile-time and compile-step validation for bus conflicts and phase rules.
- Keep the pipeline inspectable between passes for debugging.

### Non-Goals (for initial milestone)

- Full assembler integration or program loading.
- Microcode scheduling optimizations beyond safe step merging.
- Hardware-targeted ROM images beyond simulator use.

## System Overview

The microcode pipeline turns high-level instruction definitions into encoded
control words consumable by the simulator controller.

```
DSL → IR → Passes → Validated IR → Encoder → Control ROM
```

### DSL (Domain-Specific Language)

High-level builder API (C++-first, no parser initially) that references HDL
components directly:

```cpp
ir::Instruction LDA_Immediate(const hdl::Cpu& hdl) {
  return MicrocodeBuilder(hdl)
    .ReadMemoryInto(hdl.a())
    .Increment(hdl.pc())
    .Build(isa::Opcode::LDA_IMM);
}
```

Branching instructions are represented by multiple microcode programs keyed by
status conditions (see IR below). Conditions may be partial; the emitter expands
don't-care bits into all permutations.

### IR (Intermediate Representation)

```cpp
namespace microcode::ir {
  using StatusCondition = std::map<const hdl::Status*, bool>;

  struct Step {
    int stage;
    std::set<const hdl::Control*> controls;
  };

  struct Instruction {
    isa::Opcode opcode;
    StatusCondition conditions;  // Partial mapping allowed, empty means unconditional
    std::vector<Step> steps;
  };

  struct InstructionSet {
    std::vector<Instruction> instructions;
  };
}
```

### Passes

Pipeline of pure transforms/validators:

- Fix-up transforms:
  - `FetchPass`: prepend common fetch/decode sequence.
  - `SequencePass`: add sequence counter management for SC/IR.
- Validators:
  - `BusValidator`: enforce single-writer-per-bus-per-phase.
  - `PhaseValidator`: enforce control phase legality.
  - `StepValidator`: stage numbering monotonic and non-empty.
- `BranchCompletenessValidator`: if a status is used, there must be a sibling
    program that matches all other status constraints (or don't-care values)
    with that one status bit flipped.
- Optimizers/Transformers:
  - `StepMerger`: merge compatible steps (no conflicts).

After each transformer (post-fix-up), run all validators to preserve invariants.

### Encoder

Maps HDL controls to bit positions and produces ROM words:

```cpp
class ControlEncoder {
 public:
  void AddMapping(const hdl::Control* hdl_ctrl, int index);
  uint64_t Encode(const std::set<const hdl::Control*>& controls) const;
};
```

The controller uses `Encode`d words indexed by `{opcode, step, status}` where
`status` is the bit-encoded value of the HDL status register. The emitter
expands partial `StatusCondition` mappings to produce a complete table.

## Implementation Plan

### Phase 1: IR + Core Types
- Add `microcode/ir/` headers for `Step`, `Instruction`, `InstructionSet`.
- Define `ControlRef` (alias to `const hdl::Control*`), `Stage` types, and
  `StatusCondition` mapping.
- Add basic equality helpers for tests.

### Phase 2: Builder DSL (C++ API)
- Create `MicrocodeBuilder` that accumulates steps.
- Provide explicit operations: `Write(control)`, `Read(control)`, `Process(control)`.
- Add convenience helpers: `Transfer(src, dst)` and `Increment(counter)`.
- Allow `WhenStatus({hdl::Status* -> bool})` to emit conditional programs for
  branch instructions.

### Phase 3: Validation Passes
- `BusValidator`: ensure single writer per bus per phase.
- `PhaseValidator`: ensure controls only in their allowed phase.
- `StepValidator`: stage numbering monotonic and non-empty.

### Phase 4: Encoding
- `ControlEncoder` with deterministic ordering of HDL controls.
- Encode to `uint64_t` control words (validate control count limit).
- Build `InstructionRom` mapping `{opcode, step, status}` to `ControlWord`.
  - Expand partial status mappings to full permutations.

### Phase 5: Integration Points
- `microcode::Compile(InstructionSet)` entry point.
- `sim::Controller` consumes ROM and asserts sim controls.
- Add compile-time checks for HDL control counts.
- Add HDL controller components: `InstructionRegister` and `SequenceCounter`.

## HDL Requirements for Microcode

- CPU must include a status register (`SR`) as a byte register.
- `SR` owns individual `Status` wires that expose each bit as a typed signal.
- The `Status` objects act as views into `SR` (the `SR` value is authoritative).
- Intended `SR` bit layout follows 6502 conventions (N V - B D I Z C), but
  HDL remains minimal until the status register is added.
- CPU must include controller components: `IR` (instruction register) and `SC`
  (sequence counter) for fetch/decode and microcode stepping.

## Testing Plan

- Unit tests per pass: validator rejects known conflicts, accepts valid steps.
- DSL builder tests: step layout and control sets are correct.
- Encoder tests: stable bit positions, round-trip encode/decode.
- Integration tests: compile a minimal instruction set and inspect ROM output.

## Dependencies

- `irata2::base` - Byte/Word types
- `irata2::hdl` - Control signal definitions
- `irata2::isa` - Instruction set definitions

## Usage

```cmake
target_link_libraries(your_target PRIVATE irata2::microcode)
```

## Future Work

1. Optional text DSL parser.
2. Microcode optimizer beyond step merging.
3. Conditional microcode based on status flags.
4. ROM export formats for external tools.
