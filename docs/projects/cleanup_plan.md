# Cleanup Implementation Plan

This document provides a detailed implementation plan for the changes proposed in
[cleanup.md](cleanup.md). Changes are organized into phases based on dependencies.

## Overview

The cleanup effort involves significant architectural changes to the sim module:
- Component hierarchy improvements (visibility, factory patterns)
- Control and Register type redesigns
- Controller submodule with hardware-ish implementation
- Memory structure changes
- Microcode compiler/validator improvements

## Phase 1: Foundation Changes [COMPLETE]

Better encapsulation and automatic tick propagation.

### 1.1 Visibility Improvements [COMPLETE]

**Goal:** Make members protected/private unless they must be public.

| Component | Changes |
|-----------|---------|
| Component | `current_phase()`, `RegisterChild()`, `Tick*()` -> protected |
| ComponentWithParent | `RegisterChild()` -> protected (called during construction) |

**Files:**
- [component.h](../../sim/include/irata2/sim/component.h)
- [component_with_parent.h](../../sim/include/irata2/sim/component_with_parent.h)

**Steps:**
1. Add `protected:` section to Component for tick methods
2. Move `RegisterChild()` to protected in ComponentWithParent
3. Ensure tests use public interface only (refactor if needed)
4. Verify all tests pass

### 1.2 Children List in Components [COMPLETE]

**Goal:** Components maintain list of children for tick propagation.

**Changes:**
- Add `std::vector<Component*> children_` to Component
- `RegisterChild()` populates this list
- Tick methods propagate to children automatically

**Files:**
- [component.h](../../sim/include/irata2/sim/component.h)
- [cpu.h](../../sim/include/irata2/sim/cpu.h)

**Steps:**
1. Add `children_` vector to Component (protected)
2. Modify `RegisterChild()` to populate children list
3. Base `Tick*()` implementations call children's `Tick*()` methods
4. Update CPU to use children list instead of manual component vector
5. Verify tick ordering is preserved

### 1.3 Structural References as Const [COMPLETE]

**Goal:** All parent/children references are const and set during construction.

**Changes:**
- Ensure `parent_` is already const reference (verify)
- Mark `children_` as populated during construction only

**Files:**
- [component_with_parent.h](../../sim/include/irata2/sim/component_with_parent.h)

**Steps:**
1. Audit all structural references
2. Add assertions or design that prevents modification post-construction
3. Document the invariant

## Phase 2: Control Hierarchy Redesign [COMPLETE]

Clean control type hierarchy with virtual root and intermediate classes.

### 2.1 New Control Hierarchy [COMPLETE]

```
Control (root, virtual)
├── AutoResetControl
│   ├── WriteControl (Write phase)
│   ├── ReadControl (Read phase)
│   └── ProcessControl (Process phase)
└── LatchedControl
    ├── LatchedWriteControl
    ├── LatchedReadControl
    └── LatchedProcessControl
```

**Files:**
- [control.h](../../sim/include/irata2/sim/control.h) (major rewrite)
- New: `auto_reset_control.h`, `latched_control.h`

**Steps:**
1. Create abstract `Control` base with virtual interface
2. Create `AutoResetControl` intermediate class
3. Create `LatchedControl` intermediate class
4. Migrate phase-specific controls to inherit from appropriate intermediate
5. Keep phase information in templates
6. Update all control usages to use leaf types
7. Deprecate old control patterns

### 2.2 Update All Control Usages

**Affected files (grep for Control usage):**
- Cpu, Controller, Registers, Alu, Memory, StatusAnalyzer

**Steps:**
1. Identify all control declarations
2. Change to appropriate leaf type
3. Verify phase enforcement still works

## Phase 3: Register Hierarchy Redesign

**Goal:** Clean register hierarchy with ValueType template, optional bus connection.

### 3.1 Base Register Changes

**New hierarchy:**
```
Register<ValueType> (base, not connected to bus)
├── ByteRegister
├── WordRegister (has high/low ByteRegisters)
└── RegisterWithBus<ValueType> : ComponentWithBus
    ├── ByteRegisterWithBus
    └── WordRegisterWithBus
```

**Changes:**
- Default registers not connected to bus
- All registers have reset control by default
- RegisterWithBus implements ComponentWithBus

**Files:**
- [register.h](../../sim/include/irata2/sim/register.h)
- New: `register_with_bus.h`
- [byte_register.h](../../sim/include/irata2/sim/byte_register.h)
- [word_register.h](../../sim/include/irata2/sim/word_register.h)

**Steps:**
1. Create base Register without bus connection
2. Create RegisterWithBus that adds bus
3. Update ByteRegister and WordRegister
4. Update all usages

### 3.2 WordRegister Improvements

**Changes:**
- WordRegister has high/low ByteRegister members
- Word-level reset control
- Support for dual bus connection (word bus + byte bus)

**Steps:**
1. Add high/low ByteRegister members
2. Add word-level reset
3. Implement word bus read/write
4. Implement byte bus access through high/low registers

### 3.3 WordCounter Type

**Changes:**
- Word register with ByteRegister bytes
- Word-level increment control
- Handle byte overflow

**Files:**
- New: `word_counter.h`

**Steps:**
1. Extend WordRegister (or WordRegisterWithBus)
2. Add increment control (ProcessControl)
3. Implement low byte increment with overflow to high byte

### 3.4 LatchedWordRegister Redesign

**Current:** Simple value holder with no controls.

**New design:**
- Not connected to bus
- Hard connection to target WordRegister
- Latch control copies target value

**Use case:** IPC (Instruction Pointer Cache) latches from PC.

**Steps:**
1. Add constructor parameter for target register reference
2. Add latch control (ProcessControl)
3. TickProcess copies target value when latch asserted

### 3.5 CPU-Level TMP Word Register

**Goal:** Add a tmp word register connected to both buses for complex addressing.

**Use cases:**
- Moving words between registers
- Building addresses from memory bytes
- Absolute addressing mode support

**Steps:**
1. Add TMP WordRegister to CPU
2. Connect to word bus and byte bus
3. Wire up controls in controller

## Phase 4: ComponentWithBus Abstraction [COMPLETE]

Generalize bus interaction with abstract read/write methods.

### 4.1 Abstract Methods [COMPLETE]

**Changes:**
```cpp
template<typename Derived, typename ValueType>
class ComponentWithBus : public ComponentWithParent {
protected:
  // Subclasses implement these
  virtual ValueType read_value() = 0;
  virtual void write_value(ValueType value) = 0;

  // Base class implements tick methods
  void TickWrite() final {
    if (write().asserted()) {
      bus().Write(read_value(), path());
    }
  }

  void TickRead() final {
    if (read().asserted()) {
      write_value(bus().Read(path()));
    }
  }
};
```

**Files:**
- [component_with_bus.h](../../sim/include/irata2/sim/component_with_bus.h)

**Steps:**
1. Add abstract `read_value()` and `write_value()` protected methods
2. Implement TickWrite/TickRead in base class
3. Update Register to implement the abstract methods
4. Update Memory to implement the abstract methods

## Phase 5: Memory Refactoring

**Goal:** Regions and Modules as ComponentWithParent with factory pattern.

### 5.1 Region as ComponentWithParent

**Current:** Region is a simple struct with name, offset, module.

**New:** Region is ComponentWithParent.

**Initialization challenge:** Bidirectional references (Memory has Regions, Regions know parent).

**Solution:** Factory pattern - Memory constructor takes vector of region factories.

```cpp
using RegionFactory = std::function<std::unique_ptr<Region>(Memory& parent)>;

Memory::Memory(Cpu& cpu, const std::vector<RegionFactory>& region_factories)
  : ComponentWithBus(...) {
  for (const auto& factory : region_factories) {
    regions_.push_back(factory(*this));
  }
}
```

**Files:**
- [region.h](../../sim/include/irata2/sim/memory/region.h)
- [memory.h](../../sim/include/irata2/sim/memory/memory.h)

**Steps:**
1. Make Region extend ComponentWithParent
2. Change Memory constructor to accept region factories
3. Update all Memory construction sites

### 5.2 Module as ComponentWithParent

**Similar pattern:** Region takes module factory.

```cpp
using ModuleFactory = std::function<std::unique_ptr<Module>(Region& parent)>;

Region::Region(Memory& parent, const std::string& name,
               Word offset, ModuleFactory module_factory)
  : ComponentWithParent(parent, name), offset_(offset) {
  module_ = module_factory(*this);
}
```

**Files:**
- [module.h](../../sim/include/irata2/sim/memory/module.h)
- [ram.h](../../sim/include/irata2/sim/memory/ram.h)
- [rom.h](../../sim/include/irata2/sim/memory/rom.h)

**Steps:**
1. Make Module extend ComponentWithParent
2. Make Ram/Rom extend Module properly
3. Update Region to use module factory
4. Update all Region construction sites

### 5.3 Memory as ComponentWithBus

**Current:** Memory already extends ComponentWithBus.

**Verify:** Ensure it properly uses the abstraction from Phase 4.

## Phase 6: Controller Submodule

This is the largest and most complex phase. The controller becomes a proper
submodule with hardware-ish implementation.

### 6.1 Directory Structure

```
sim/
├── include/irata2/sim/
│   └── controller/
│       ├── controller.h
│       ├── control_encoder.h
│       ├── status_encoder.h
│       ├── instruction_encoder.h
│       ├── instruction_memory.h
│       └── rom_grid.h
└── src/
    └── controller/
        ├── controller.cpp
        ├── control_encoder.cpp
        ├── status_encoder.cpp
        ├── instruction_encoder.cpp
        └── instruction_memory.cpp
```

### 6.2 ControlEncoder

**Purpose:** Encode/decode control signals to/from binary control words.

**Interface:**
```cpp
class ControlEncoder : public ComponentWithParent {
public:
  // At startup: extract all controls from microcode, fetch sim references
  void Initialize(const MicrocodeProgram& program, Cpu& cpu);

  // Encode: set of control references -> binary control word
  std::vector<uint8_t> Encode(const std::vector<ControlBase*>& controls) const;

  // Decode: binary control word -> set of control references
  std::vector<ControlBase*> Decode(const std::vector<uint8_t>& word) const;

private:
  std::vector<ControlBase*> control_references_;  // Ordered list
  size_t control_word_width_;  // Bytes needed to encode all controls
};
```

**Steps:**
1. Create ControlEncoder class
2. Implement control enumeration from MicrocodeProgram
3. Implement encoding logic (control index -> bit position)
4. Implement decoding logic (bit position -> control reference)

### 6.3 StatusEncoder

**Purpose:** Encode/decode status values for ROM addressing. Handles expansion
of sparse microcode (partial status requirements) into dense ROM entries.

**Key concepts:**
- `CompleteStatus`: All status bits have defined values (e.g., C=1, Z=0, N=0, V=1)
- `PartialStatus`: Only some status bits specified, others are "don't care"
  (e.g., C=1, Z=*, N=*, V=*) - this is what microcode steps output

**Interface:**
```cpp
class StatusEncoder : public ComponentWithParent {
public:
  void Initialize(const MicrocodeProgram& program, Cpu& cpu);

  // Complete status (all bits specified) -> binary encoding for ROM address
  uint32_t Encode(const CompleteStatus& status) const;

  // Binary encoding -> complete status
  CompleteStatus Decode(uint32_t encoded) const;

  // Expand partial status to all matching complete statuses
  // Used at construction time to populate ROM for all "don't care" combinations
  std::vector<CompleteStatus> Permute(const PartialStatus& partial) const;

private:
  std::vector<Status*> status_references_;  // Ordered list for stable encoding
};
```

**Steps:**
1. Create StatusEncoder class
2. Implement status enumeration from MicrocodeProgram
3. Implement encoding/decoding for ROM addressing
4. Implement PartialStatus permutation for sparse-to-dense expansion

### 6.4 InstructionMemory

**Purpose:** Hardware-ish ROM storage for microcode.

**Design:**
- Contains ControlEncoder and StatusEncoder
- Contains grid of Rom objects (16-bit address space, 8-bit data each)
- Dynamically sizes ROM grid based on instruction/control space
- Takes microcode program at construction, encodes into ROMs, discards program

**Interface:**
```cpp
class InstructionMemory : public ComponentWithParent {
public:
  InstructionMemory(Controller& parent, const MicrocodeProgram& program);

  // Lookup: (opcode, step, status) -> control references
  std::vector<ControlBase*> Lookup(uint8_t opcode, uint8_t step,
                                    const CompleteStatus& status) const;

private:
  ControlEncoder control_encoder_;
  StatusEncoder status_encoder_;
  std::vector<std::vector<Rom>> rom_grid_;  // [row][col]
  size_t address_width_;
  size_t data_width_;
};
```

**Steps:**
1. Create InstructionMemory class
2. Implement ROM grid sizing calculation
3. Implement microcode-to-ROM encoding
4. Implement lookup logic

### 6.5 Controller Refactoring

**Current:** Stores MicrocodeProgram directly, accesses it at runtime.

**New:** Uses InstructionMemory for lookups.

**Changes:**
```cpp
class Controller : public ComponentWithParent {
public:
  Controller(Cpu& cpu, const MicrocodeProgram& program);

  void TickControl() override {
    // Get current state
    uint8_t opcode = ir_.value().value();
    uint8_t step = sc_.value().value();
    CompleteStatus status = GatherStatus();

    // Lookup in InstructionMemory
    auto controls = instruction_memory_.Lookup(opcode, step, status);

    // Assert controls
    for (auto* control : controls) {
      control->Assert();
    }
  }

private:
  ByteRegister ir_;
  LocalCounter<base::Byte> sc_;
  LatchedWordRegister ipc_;
  InstructionMemory instruction_memory_;  // Owns the encoders and ROMs
};
```

**Steps:**
1. Refactor Controller to use InstructionMemory
2. Remove direct MicrocodeProgram storage
3. Update Controller tests

## Phase 7: CPU Constructor Refactoring

**Goal:** CPU should statically lookup HDL and compiled microcode.

### 7.1 Static HDL/Microcode Singletons

**Current:** CPU accepts HDL and MicrocodeProgram as constructor args.

**New:** Private static methods build singletons.

```cpp
class Cpu : public Component {
public:
  // New constructor: just needs ROM and optional debug symbols
  Cpu(Rom cartridge,
      std::optional<DebugSymbols> debug_symbols = std::nullopt,
      std::vector<RegionFactory> extra_regions = {});

private:
  // Singleton builders (replaces initialization.h)
  static const Hdl& GetHdl();
  static const MicrocodeProgram& GetMicrocodeProgram();

  // Validate sim matches HDL
  void ValidateAgainstHdl(const Hdl& hdl);
};
```

**Steps:**
1. Create static singleton methods for HDL and MicrocodeProgram
2. Move initialization logic from initialization.h into CPU
3. Add validation method
4. Update CPU constructor to use singletons
5. Update all CPU construction sites (tests may need adjustment)

### 7.2 RunResult Improvements

**Current:** RunResult is minimal.

**New:** Full debug dump and halt reason.

```cpp
enum class HaltReason {
  Timeout,
  Halt,
  Crash
};

struct CpuState {
  uint8_t a, x;
  uint16_t pc;
  uint8_t ir, sc;
  // ... all registers and status flags
};

struct RunResult {
  HaltReason reason;
  uint64_t cycles;
  std::optional<CpuState> state;  // Full dump when requested
};
```

**Steps:**
1. Add HaltReason enum
2. Add CpuState struct
3. Update RunResult
4. Update RunUntilHalt to populate new fields

## Phase 8: HDL Enforcement

**Goal:** HDL should enforce that sim is a superset of HDL at startup.

### 8.1 Validation Method

**Add to HDL:** Method to validate against sim.

```cpp
class Hdl {
public:
  // Throws if sim doesn't match HDL structure
  void ValidateAgainst(const Cpu& cpu) const;
};
```

**Checks:**
- All HDL components exist in sim
- All HDL controls exist in sim
- All HDL buses exist in sim
- Types match

**Steps:**
1. Add validation method to HDL
2. Call during CPU construction
3. Handle structural changes (new hierarchy) in HDL

### 8.2 HDL Structural Updates

Update HDL to reflect new sim structure:
- New control hierarchy
- New register hierarchy
- Controller submodule structure
- Memory region/module hierarchy

## Phase 9: Documentation and Cleanup

### 9.1 One Class Per File

Audit and fix any files with multiple classes.

### 9.2 Directory Nesting

Ensure submodules have proper directory structure:
- `sim/alu/` for ALU components
- `sim/controller/` for controller components
- `sim/memory/` for memory components

### 9.3 Namespace Nesting

Update namespaces to match directory structure:
- `irata2::sim::alu::Alu`
- `irata2::sim::controller::Controller`
- `irata2::sim::memory::Memory`

### 9.4 Documentation

Each top-level module should have `docs/` directory with:
- `readme.md` - Module overview
- Design doc for each submodule

## Phase 10: Microcode Module Cleanup [COMPLETE]

**Goal:** Improve validators, add step-merging optimizer, restructure compiler.

**Status:** All items complete (10.2, 10.3, 10.4, 10.5). BusValidator improvements (10.1)
deferred until HDL type information is available.

### 10.1 BusValidator Improvements

**Current:** Uses hardcoded lookups to determine register types.

**Problem:** The type system already knows this. The HDL has control references with
bus information.

**Solution:** Use HDL type information instead of hardcoding.

**Changes:**
- MicrocodeProgram has control references into HDL
- HDL knows which controls are read/write controls with bus references
- BusValidator should use this information for bus deconfliction
- If HDL is missing this info, update HDL first

**Files:**
- [bus_validator.cpp](../../microcode/src/compiler/bus_validator.cpp)
- May require HDL updates

**Steps:**
1. Audit HDL for bus/control type information
2. Add missing type info to HDL if needed
3. Refactor BusValidator to use HDL types instead of hardcoded lookups
4. Remove hardcoded register name checks

### 10.2 ControlConflictValidator Fix [COMPLETE]

**Current:** Flags multiple ALU opcode bits as a conflict.

**Problem:** ALU opcode is binary encoded - multiple bits being set is expected.

**Solution:** Don't treat ALU opcode bits as conflicting controls.

**Files:**
- [control_conflict_validator.cpp](../../microcode/src/compiler/control_conflict_validator.cpp)

**Steps:**
1. Identify ALU opcode controls
2. Exclude them from conflict detection (they're binary encoded, not one-hot)

### 10.3 PhaseOrderingValidator Removal [COMPLETE]

**Current:** Validates control ordering within a step.

**Problem:** This validator is based on a misunderstanding of tick phases.

**Key insight:** Within each tick phase, components are ticked in a **nondeterministic
order** and controls are consumed in a **nondeterministic order**. This is by design.
Controls enforce access to their values in the sim to protect tick phase reasoning.

**Solution:** Remove this validator entirely - it's not needed.

**Files:**
- [phase_ordering_validator.h](../../microcode/include/irata2/microcode/compiler/phase_ordering_validator.h)
- [phase_ordering_validator.cpp](../../microcode/src/compiler/phase_ordering_validator.cpp)
- [phase_ordering_validator_test.cpp](../../microcode/test/phase_ordering_validator_test.cpp)
- [compiler.cpp](../../microcode/src/compiler/compiler.cpp)

**Steps:**
1. Remove PhaseOrderingValidator from compiler
2. Delete PhaseOrderingValidator files
3. Update documentation to clarify nondeterministic ordering within phases

### 10.4 Step-Merging Optimizer [COMPLETE]

**Goal:** Merge adjacent steps when safe based on phase ordering.

**Phase ordering rules:**
- Tick phases are ordered: `control < write < read < process < clear`
- All controls in a step happen in tick phase order (control *set* is unordered
  except for phases)
- Control a ≤ control b if control a's phase ≤ control b's phase
- Step a ≤ step b if all controls in step a ≤ all controls in step b
- Step a is mergeable with step b if step a ≤ step b AND they're in the same stage

**Short version:** Steps are mergeable if everything in step a happens before or at
the same time as everything in step b, and they're in the same stage.

**Files:**
- New: `step_merging_optimizer.h`, `step_merging_optimizer.cpp`
- New: `step_merging_optimizer_test.cpp`

**Steps:**
1. Implement phase comparison for controls
2. Implement step comparison (a ≤ b)
3. Implement merge detection (same stage + a ≤ b)
4. Implement step merging (union of control sets)
5. Add comprehensive tests

### 10.5 Compiler Restructuring [COMPLETE]

**Goal:** Defensive compiler structure with preamble, validators, transformers.

**New structure:**
1. **Preamble passes** - Required to get microcode to valid state
   - StepTransformer
   - SequenceTransformer
2. **Validators** - All validators run after preamble
3. **Transformers** - Each transformer followed by all validators

**Pattern:** For each transformer:
1. Run transformer
2. Run all validators
3. If any validator fails, transformer produced invalid output

**Current issue:** End of `Compiler::Compile` does encoding inline. This should be
a separate encoder class.

**Files:**
- [compiler.h](../../microcode/include/irata2/microcode/compiler/compiler.h)
- [compiler.cpp](../../microcode/src/compiler/compiler.cpp)
- New: `encoder.h`, `encoder.cpp` (for final encoding step)

**Steps:**
1. Categorize existing passes as preamble/validator/transformer
2. Implement new compilation loop with defensive validation
3. Extract encoding logic into separate Encoder class
4. Add tests for invalid transformer output detection

## Dependency Graph

```
Phase 1 (Foundation)
    ↓
Phase 2 (Controls) ←──────────┐
    ↓                         │
Phase 3 (Registers)           │
    ↓                         │
Phase 4 (ComponentWithBus)    │
    ↓                         │
Phase 5 (Memory)              │
    ↓                         │
Phase 6 (Controller) ─────────┘
    ↓
Phase 7 (CPU Constructor)
    ↓
Phase 8 (HDL Enforcement) ←── Phase 10 (Microcode) [10.1 needs HDL types]
    ↓
Phase 9 (Documentation)
```

Notes:
- Phase 6 depends on Phase 2 because the ControlEncoder needs the new control hierarchy.
- Phase 10 (Microcode) is mostly independent and can be done early, except 10.1
  (BusValidator) which needs HDL type information from Phase 8.
- Phase 10.2, 10.3, 10.4, 10.5 can be done at any time.

## Estimated Complexity

| Phase | Files Changed | New Files | Risk Level |
|-------|---------------|-----------|------------|
| 1     | 3-5           | 0         | Low        |
| 2     | 10-15         | 2-3       | Medium     |
| 3     | 8-12          | 3-4       | Medium     |
| 4     | 3-5           | 0         | Low        |
| 5     | 5-8           | 0         | Medium     |
| 6     | 5-10          | 8-10      | High       |
| 7     | 3-5           | 0         | Medium     |
| 8     | 5-8           | 0         | Medium     |
| 9     | Many          | Several   | Low        |
| 10    | 5-8           | 3-4       | Medium     |

## Design Decisions

These decisions were clarified during planning and should guide implementation.

### Factory Pattern Style

Use `std::function` for all factories:
```cpp
using RegionFactory = std::function<std::unique_ptr<Region>(Memory& parent)>;
using ModuleFactory = std::function<std::unique_ptr<Module>(Region& parent)>;
```

### Testing Strategy

**Full CPU testing only.** Tests should use the complete CPU with real HDL and microcode,
not custom configurations. This is a project-level consistency decision.

To maintain test quality:
- Build test harnesses, matchers, and helpers as needed
- Keep tests easy to understand despite full CPU complexity
- Accept slight coverage reduction as a conscious tradeoff for clean class design
- Same applies to visibility changes: prefer clean design over testability when in conflict

### InstructionMemory Sizing

InstructionMemory dynamically sizes its ROM grid based on the microcode program:
- Calculate required address space from opcode/step/status dimensions
- Calculate required data width from control word size
- Generate appropriate number of 16-bit address / 8-bit data ROMs

### WordRegister Dual Bus Connection

WordRegisters support simultaneous connection to both buses:
- The word as a whole moves on the **word bus** (typically address bus)
- The high/low bytes individually move on the **byte bus** (typically data bus)
- Both can happen in the same tick - phase ordering makes this safe
- Bus contention is handled by the existing phase model

Example use case: MAR can receive a full word from PC (word bus) or be loaded
byte-by-byte from memory (byte bus).

### Controller Encoder Timing

All encoding happens at construction time:
1. Controller receives MicrocodeProgram
2. Encoders process program and build encoding tables
3. InstructionMemory encodes all microcode into ROM grid
4. MicrocodeProgram reference is discarded
5. Runtime behavior uses only InstructionMemory/ROM lookups

The microcode program output is **sparse** - it only specifies statuses the
instruction "cares about" for each step. The StatusEncoder's `Permute()` function
expands these partial status requirements into all matching complete statuses,
ensuring the ROM grid is fully addressed. For example, if a step only cares about
the carry flag being set, permutation generates entries for all combinations of
the other status bits (zero, negative, overflow, etc.).

### Phase Implementation Order

Phases should follow the dependency graph but can be interleaved with other
project work. Prioritize stability and testing. Independent phases (e.g.,
Phase 3 Registers and Phase 5 Memory) can be worked in parallel if desired.
