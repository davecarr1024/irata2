# IRATA2 Design Document

**Date**: 2026-01-07
**Project**: IRATA2 - 8-bit CPU Simulator in C++
**Philosophy**: Hardware-ish. Breadboard in software.

---

## Vision

Build a cycle-accurate 8-bit CPU simulator in C++ that feels like building a breadboard computer. Components compose like hardware modules. The system is testable, incremental, and provably correct at every step. The long-term dream: a vector graphics Asteroids game running on a 2600-style system with memory-mapped I/O, written entirely in assembly.

This is a learning project. The goal is to deeply understand CPU design and implementation through building a complete system from first principles: HDL → microcode → simulator → assembler → game.

---

## Core Philosophy

### Hardware-ish

Components are **dumb hardware simulators**. No magic, minimal logic. When implementing an ALU operation, we're pretending to implement logic gates. Components don't know about the instruction set or program semantics - they're just pretend hardware that responds to control signals.

**All smarts live in the microcode**, not in component implementations.

### Strongly Typed, Immutable Structure

The component tree is **not a dynamic game engine**. It's a **single, purpose-built CPU**. We're not building a CPU simulation framework - we're building one specific CPU. This means:

- Components are strongly typed members, not dynamic pointers
- The tree structure is immutable after construction
- Invalid states are unrepresentable in the type system
- Navigation is type-safe: `cpu().memory().mar()` not `get_child("/cpu/memory/mar")`
- No generality unless needed for the IRATA instruction set
- Parent links are type-erased to maximize component reuse across different owners

### Defense in Depth

Prevent bugs through multiple validation layers:

1. **HDL validation** (compile-time) - Can't construct disconnected components
2. **Microcode validation** (post-optimization) - Can't create bus conflicts
3. **Runtime validation** (during execution) - Can't violate phase rules

Each layer catches different bug classes. Layers 1 and 2 catch bugs **before simulation runs**.

### Test-Driven, Incremental Progress

- 100% test coverage (like PIRATA)
- Test infrastructure is critical - custom matchers, harnesses, parameterized tests
- Make tests readable by hiding complexity in infrastructure
- Every milestone produces a working, testable system
- Frequent early wins to maintain momentum

---

## System Architecture

### The Three Representations

The system has three parallel representations of the CPU:

#### 1. HDL (Hardware Definition Language)

**Purpose**: Structural metadata. The contract that microcode programs against.

**Properties**:
- Immutable after construction
- No runtime state
- Pure structural description
- Strongly typed component hierarchy
- Components are const members (value types)

**Example**:
```cpp
namespace hdl {
  class Cpu : public Component {
  public:
    Cpu()
      : data_bus_("data_bus", *this),
        a_("a", *this, data_bus_),
        x_("x", *this, data_bus_) {}

    const ByteBus& data_bus() const { return data_bus_; }
    const ByteRegister& a() const { return a_; }
    const ByteRegister& x() const { return x_; }

    // Override from Component
    Cpu& cpu() override { return *this; }

  private:
    const ByteBus data_bus_;
    const ByteRegister a_;
    const ByteRegister x_;
  };
}
```

#### 2. Microcode IR (Intermediate Representation)

**Purpose**: Compiled instruction programs. Maps (opcode, step, status) → control words.

**Flow**:
```
DSL → IR → Compiler Passes → Validated IR → Encoder → Instruction Memory ROM
```

**Properties**:
- Immutable after compilation
- References HDL components (structural)
- Multi-pass compilation (transforms, validators, optimizers)
- Inspectable between passes

**Example**:
```cpp
namespace microcode::ir {
  struct Step {
    int stage;
    std::set<const hdl::Control*> controls;  // References HDL
  };

  struct Instruction {
    uint8_t opcode;
    std::vector<Step> steps;
    std::map<const hdl::Status*, bool> conditions;
  };
}
```

#### 3. Simulator (Runtime Execution)

**Purpose**: Hardware simulation. Actual register values, bus values, control states.

**Properties**:
- Matches HDL structure (validated at construction)
- Has runtime state (register values, bus values)
- Implements five-phase tick model
- Components are dumb hardware (no instruction logic)

**Example**:
```cpp
namespace sim {
  class ByteRegister : public ComponentWithParent {
  public:
    ByteRegister(Component& parent, const std::string& name, ByteBus& bus)
      : ComponentWithParent(parent),
        name_(name),
        bus_(bus),
        write_control_("write", *this, TickPhase::Write),
        read_control_("read", *this, TickPhase::Read),
        value_(0x00) {}

    void TickWrite() override {
      if (write_control_.value()) {
        bus_.Write(value_);
      }
    }

    void TickRead() override {
      if (read_control_.value()) {
        value_ = bus_.Read();
      }
    }

    WriteControl& write() { return write_control_; }
    uint8_t value() const { return value_; }

  private:
    std::string name_;
    ByteBus& bus_;
    WriteControl write_control_;   // Note: not const (runtime state)
    ReadControl read_control_;
    uint8_t value_;                 // Runtime state
  };
}
```

---

## Component Model

### Type Hierarchy

```cpp
Component (abstract)
├── Cpu (root, exactly one per tree)
└── ComponentWithParent (all others)
    ├── Bus<T>
    │   ├── ByteBus
    │   └── WordBus
    ├── Control (phase-aware leaf components)
    │   ├── WriteControl
    │   ├── ReadControl
    │   └── ProcessControl
    ├── Status (flag components)
    ├── Register
    │   ├── ByteRegister
    │   └── WordRegister (composed of 2 ByteRegisters)
    ├── Memory
    ├── Alu
    │   └── AluOperation (modules for add, sub, etc)
    └── Controller
        ├── InstructionRegister
        ├── SequenceCounter
        └── InstructionMemory
```

### Base Classes

```cpp
// Abstract base - every component implements this
class Component {
public:
  virtual ~Component() = default;

  // Every component can access the root
  virtual Cpu& cpu() = 0;
  virtual const Cpu& cpu() const = 0;

  // Tick phases (Cpu orchestrates, components override)
  virtual void TickControl() {}
  virtual void TickWrite() {}
  virtual void TickRead() {}
  virtual void TickProcess() {}
  virtual void TickClear() {}
};

// All non-root components (parent is type-erased for reuse)
class ComponentWithParent : public Component {
public:
  ComponentWithParent(Component& parent) : parent_(parent) {}

  Component& parent() { return parent_; }
  const Component& parent() const { return parent_; }

  Cpu& cpu() override { return parent_.cpu(); }
  const Cpu& cpu() const override { return parent_.cpu(); }

private:
  Component& parent_;  // Non-owning reference
};

// The root
class Cpu : public Component {
public:
  Cpu& cpu() override { return *this; }
  const Cpu& cpu() const override { return *this; }

  // Cpu owns all children via const members
};
```

### Ownership Rules

1. **Parents own children** as const members (value types)
2. **Children reference parents** via `ComponentWithParent` base
3. **Children reference siblings** via constructor parameters (references)
4. **No pointers** to owned components (use references)
5. **Root is always Cpu** (enforced by construction)

**Construction Order**:
```cpp
class Cpu {
  Cpu()
    : data_bus_("data_bus", *this),          // Construct bus first
      a_("a", *this, data_bus_),             // Pass bus reference
      x_("x", *this, data_bus_) {}           // Pass bus reference

private:
  const ByteBus data_bus_;      // Constructed first (member order)
  const ByteRegister a_;        // Can reference data_bus_
  const ByteRegister x_;        // Can reference data_bus_
};
```

### Navigation Pattern

**No string paths**. All navigation is strongly typed:

```cpp
// Getting the MAR from anywhere in the component tree
void SomeComponent::DoSomething() {
  auto& mar = cpu().memory().mar();  // Strongly typed, compile-time checked
  // vs old way: cpu().get_child("/cpu/memory/mar")
}
```

**Tradeoff**: Tests must construct full Cpu. But tests always test production configuration.

---

## Composability

Components compose hierarchically like hardware modules:

### Example: WordRegister

```cpp
class WordRegister : public ComponentWithParent {
public:
  WordRegister(Component& parent, const std::string& name)
    : ComponentWithParent(parent),
      high_("high", *this),
      low_("low", *this) {}

  const ByteRegister& high() const { return high_; }
  const ByteRegister& low() const { return low_; }

  uint16_t value() const {
    return (static_cast<uint16_t>(high_.value()) << 8) | low_.value();
  }

private:
  const ByteRegister high_;  // Composed from smaller components
  const ByteRegister low_;
};
```

### Example: Control as Leaf Component

Controls are basic leaf components that hang off their parent:

```cpp
class ByteRegister : public ComponentWithParent {
public:
  ByteRegister(Component& parent, const std::string& name, ByteBus& bus)
    : ComponentWithParent(parent),
      bus_(bus),
      write_control_("write", *this, TickPhase::Write),
      read_control_("read", *this, TickPhase::Read) {}

  const WriteControl& write() const { return write_control_; }

private:
  ByteBus& bus_;
  const WriteControl write_control_;  // Leaf component
  const ReadControl read_control_;    // Leaf component
};
```

Think of it as a box diagram: the register is a box containing control connectors and a byte value.

---

## Five-Phase Tick Model

Every clock cycle executes five phases in strict order:

```
┌─────────────────────────────────────┐
│ 1. Control Phase                    │
│    - Controller reads microcode ROM │
│    - Asserts control signals        │
├─────────────────────────────────────┤
│ 2. Write Phase                      │
│    - Components write to buses      │
│    - Bus enforces single-writer     │
├─────────────────────────────────────┤
│ 3. Read Phase                       │
│    - Components read from buses     │
│    - Bus enforces read-after-write  │
├─────────────────────────────────────┤
│ 4. Process Phase                    │
│    - Internal component updates     │
│    - ALU operations                 │
│    - Status flag updates            │
├─────────────────────────────────────┤
│ 5. Clear Phase                      │
│    - Reset auto-clear controls      │
│    - Clear transient state          │
└─────────────────────────────────────┘
```

**Phase Enforcement**:
```cpp
class Cpu {
public:
  void Tick() {
    current_phase_ = TickPhase::Control;
    TickControl();

    current_phase_ = TickPhase::Write;
    TickWrite();

    current_phase_ = TickPhase::Read;
    TickRead();

    current_phase_ = TickPhase::Process;
    TickProcess();

    current_phase_ = TickPhase::Clear;
    TickClear();

    current_phase_ = TickPhase::None;
  }

  TickPhase current_phase() const { return current_phase_; }

private:
  TickPhase current_phase_ = TickPhase::None;
};
```

**Phase-Aware Access**:
```cpp
class ByteBus {
  void Write(Byte value) {
    if (cpu().current_phase() != TickPhase::Write) {
      throw PhaseViolationError(
        std::format("Bus {} written during {} phase (expected Write)",
                    path(), ToString(cpu().current_phase())));
    }

    if (has_writer_this_tick_) {
      throw BusConflictError(
        std::format("Bus {} already written this tick", path()));
    }

    value_ = value;
    has_writer_this_tick_ = true;
  }
};
```

**Why Phase-Aware Controls Matter**:

Controls can only be read during their assigned phase. This enables microcode optimization:

- `mem.read` (Read phase) and `a.write` (Write phase) don't conflict
- Optimizer can merge them into same step even though both touch `data_bus`
- Without phase info, optimizer would be overly conservative

---

## Microcode System

### The "Spark" - How Microcode Brings Hardware to Life

This is the magic moment: microcode (brain) + hardware (brawn) = execution.

**The Flow**:

```
1. Build HDL (structural metadata)
   ↓
2. Write microcode DSL (programs against HDL)
   ↓
3. Compile to IR (transform, validate, optimize)
   ↓
4. Encode to instruction memory ROM
   ↓
5. Build simulator (matches HDL structure)
   ↓
6. Load ROM into controller
   ↓
7. Execute: Controller reads ROM, asserts controls, hardware responds
```

### DSL Layer

```cpp
// Microcode programs reference HDL components
ir::Instruction LDA_Immediate(const hdl::Cpu& hdl) {
  return MicrocodeBuilder(hdl)
    .ReadMemoryInto(hdl.a())        // References HDL register
    .AnalyzeStatus(hdl.status())    // References HDL status
    .Build();
}
```

### IR Layer

```cpp
namespace microcode::ir {
  struct Step {
    int stage;
    std::set<const hdl::Control*> controls;  // Pointers to HDL controls
  };

  struct Instruction {
    uint8_t opcode;
    std::vector<Step> steps;
    std::map<const hdl::Status*, bool> status_conditions;
  };

  struct InstructionSet {
    std::vector<Instruction> instructions;
  };
}
```

### Compiler Passes

Multi-pass pipeline (from PIRATA):

```cpp
class Pass {
public:
  virtual ~Pass() = default;
  virtual ir::InstructionSet Transform(ir::InstructionSet ir) const = 0;
};

// Transformation passes
class FetchPass : public Pass;        // Add fetch stage
class SequencePass : public Pass;     // Add sequence counter mgmt

// Validation passes
class BusValidator : public Pass;     // Verify single-writer rule
class StepValidator : public Pass;    // Verify step numbering
class StatusValidator : public Pass;  // Verify status completeness

// Optimization passes
class StepMerger : public Pass;       // Merge compatible steps

// Compiler orchestrates
ir::InstructionSet Compile(ir::InstructionSet dsl_output) {
  auto ir = dsl_output;
  ir = FetchPass().Transform(std::move(ir));
  ir = SequencePass().Transform(std::move(ir));
  ir = BusValidator().Transform(std::move(ir));  // Throws on conflict
  ir = StepMerger().Transform(std::move(ir));
  return ir;
}
```

### Encoding - HDL to Simulator Mapping

**The Challenge**: Microcode IR references `hdl::Control*`, but simulator has `sim::Control*`. How to map?

**Solution**: Parallel traversal during encoding.

```cpp
class ControlEncoder {
public:
  ControlEncoder(const hdl::Cpu& hdl, sim::Cpu& sim) {
    hdl.MapControls(sim, *this);  // Build mapping via traversal
  }

  void AddMapping(const hdl::Control* hdl_ctrl, sim::Control* sim_ctrl) {
    hdl_to_sim_[hdl_ctrl] = sim_ctrl;
    control_index_[hdl_ctrl] = next_bit_++;
  }

  // Encode set of HDL controls → bit pattern
  uint64_t Encode(const std::set<const hdl::Control*>& controls) {
    uint64_t word = 0;
    for (auto* ctrl : controls) {
      word |= (1ULL << control_index_.at(ctrl));
    }
    return word;
  }

  // Decode bit pattern → vector of sim controls
  std::vector<sim::Control*> Decode(uint64_t word) {
    std::vector<sim::Control*> controls;
    for (const auto& [hdl_ctrl, sim_ctrl] : hdl_to_sim_) {
      if (word & (1ULL << control_index_.at(hdl_ctrl))) {
        controls.push_back(sim_ctrl);
      }
    }
    return controls;
  }

private:
  std::map<const hdl::Control*, sim::Control*> hdl_to_sim_;
  std::map<const hdl::Control*, int> control_index_;
  int next_bit_ = 0;
};
```

**Mapping via parallel traversal**:

```cpp
namespace hdl {
  class ByteRegister {
    void MapControls(sim::ByteRegister& sim_reg, ControlEncoder& encoder) const {
      encoder.AddMapping(&write_control_, &sim_reg.write());
      encoder.AddMapping(&read_control_, &sim_reg.read());
    }
  };

  class Cpu {
    void MapControls(sim::Cpu& sim_cpu, ControlEncoder& encoder) const {
      a_.MapControls(sim_cpu.a(), encoder);
      x_.MapControls(sim_cpu.x(), encoder);
      memory_.MapControls(sim_cpu.memory(), encoder);
      // ... all components
    }
  };
}
```

**Strongly typed, happens once at construction, no string paths!**

### Instruction Memory ROM

```cpp
struct InstructionAddress {
  uint8_t opcode;
  uint8_t step;
  uint8_t status;  // Status flags as bit pattern
};

struct ControlWord {
  uint64_t controls;  // Encoded control signals
};

class InstructionMemory {
public:
  const ControlWord& Read(InstructionAddress addr) const {
    return memory_.at(addr);
  }

private:
  std::map<InstructionAddress, ControlWord> memory_;
};
```

### Controller Execution

```cpp
class Controller : public ComponentWithParent {
public:
  void TickControl() override {
    // Read current instruction state
    InstructionAddress addr{
      .opcode = ir_.value(),
      .step = sequence_counter_.value(),
      .status = cpu().status().encode()
    };

    // Read microcode ROM
    auto control_word = instruction_memory_.Read(addr);

    // Decode and assert controls
    auto controls = control_encoder_.Decode(control_word.controls);
    for (auto* ctrl : controls) {
      ctrl->Set(true);
    }
  }

private:
  InstructionRegister ir_;
  SequenceCounter sequence_counter_;
  InstructionMemory instruction_memory_;
  ControlEncoder control_encoder_;
};
```

---

## Testing Strategy

### Principles

1. **100% coverage** (like PIRATA) - all production code must be tested
2. **Tests are documentation** - reading tests should teach the system
3. **Infrastructure hides complexity** - custom matchers, builders, harnesses
4. **Test in production context** - always test components as part of full Cpu

### Test Infrastructure

**Custom Matchers**:
```cpp
// test/matchers.h
MATCHER_P(RegisterHasValue, expected, "") {
  return arg.value() == expected;
}

MATCHER_P(BusHasValue, expected, "") {
  return arg.Read() == Byte(expected);
}

MATCHER_P2(HasFlags, zero, negative, "") {
  return arg.zero() == zero && arg.negative() == negative;
}
```

**Test Harness**:
```cpp
class CpuTestHarness {
public:
  CpuTestHarness() : hdl_(), cpu_(hdl_) {}

  CpuTestHarness& WithProgram(const std::vector<uint8_t>& bytes) {
    cpu_.memory().Load(0x0000, bytes);
    return *this;
  }

  CpuTestHarness& WithRegister(const std::string& name, uint8_t value) {
    // ... set register value
    return *this;
  }

  void RunUntilHalt() {
    while (!cpu_.halted() && cpu_.cycle_count() < max_cycles_) {
      cpu_.Tick();
    }
  }

  sim::Cpu& cpu() { return cpu_; }

private:
  hdl::Cpu hdl_;
  sim::Cpu cpu_;
  uint64_t max_cycles_ = 1'000'000;
};
```

### Test Examples

**Unit Test** (component in context):
```cpp
TEST(ByteRegisterTest, WriteToBus) {
  CpuTestHarness harness;

  harness.cpu().a().SetValue(0x42);
  harness.cpu().a().write().Set(true);
  harness.cpu().Tick();

  EXPECT_THAT(harness.cpu().data_bus(), BusHasValue(0x42));
  EXPECT_THAT(harness.cpu().a(), RegisterHasValue(0x42));
}
```

**Integration Test** (full instruction):
```cpp
TEST(InstructionTest, LDA_Immediate) {
  auto harness = CpuTestHarness()
    .WithProgram({0xA0, 0x42});  // LDA #$42

  harness.RunUntilHalt();

  EXPECT_THAT(harness.cpu().a(), RegisterHasValue(0x42));
  EXPECT_THAT(harness.cpu().status(), HasFlags(Zero(false), Negative(false)));
}
```

**Parameterized Tests** (all .asm programs):
```cpp
class AsmProgramTest : public ::testing::TestWithParam<std::filesystem::path> {};

TEST_P(AsmProgramTest, ExecutesCorrectly) {
  auto asm_file = GetParam();
  auto binary = Assemble(asm_file);

  auto harness = CpuTestHarness().WithProgram(binary);
  harness.RunUntilHalt();

  if (asm_file.filename() == "crash.asm") {
    EXPECT_TRUE(harness.cpu().crashed());
  } else {
    EXPECT_TRUE(harness.cpu().halted());
  }
}

INSTANTIATE_TEST_SUITE_P(
  AllPrograms,
  AsmProgramTest,
  ::testing::ValuesIn(FindAllAsmFiles("test/programs/")));
```

---

## Implementation Phases

Each phase produces a working, testable milestone. Focus on frequent early wins.

### Phase 0: Project Setup & Test Infrastructure

**Goal**: Build system works, can write and run tests, code generation ready.

**Tasks**:
1. Create directory structure
   ```
   irata2/
   ├── hdl/              # HDL components
   ├── microcode/        # DSL, IR, compiler, encoder
   ├── sim/              # Runtime simulator
   ├── asm/              # ISA definitions (YAML)
   ├── assembler/        # Python assembler
   ├── test/             # Test harnesses, matchers, .asm programs
   └── tools/            # Code generators
   ```

2. Setup CMake build
   - C++20 with Clang
   - GoogleTest/GoogleMock integration
   - Coverage reporting (lcov)
   - Python integration

3. **Build test infrastructure FIRST**
   - Custom matchers library
   - `CpuTestHarness` class
   - Parameterized test utilities
   - Make tests readable before writing production code

4. Code generation
   - YAML → C++ (ISA definitions)
   - YAML → Python (assembler bindings)

**Milestone**: `make test` runs and passes (even if empty). Infrastructure exists.

**Win**: Can write beautiful, readable tests.

---

### Phase 1: HDL Foundation

**Goal**: Can construct immutable HDL tree representing CPU structure.

**Tasks**:
1. Core types: `Byte`, `Word`
2. Base classes: `Component`, `ComponentWithParent`
3. Basic components: `Bus<T>`, `Control`, `Status`
4. Simple composition: `ByteRegister` (has controls)
5. Complex composition: `WordRegister` (has two ByteRegisters)
6. Root: `hdl::Cpu` (owns all components)

**Tests**:
```cpp
TEST(HdlTest, CpuHasDataBus) {
  hdl::Cpu cpu;
  EXPECT_EQ(cpu.data_bus().name(), "data_bus");
}

TEST(HdlTest, RegisterConnectedToBus) {
  hdl::Cpu cpu;
  EXPECT_EQ(&cpu.a().bus(), &cpu.data_bus());
}

TEST(HdlTest, RegisterHasControls) {
  hdl::Cpu cpu;
  EXPECT_EQ(cpu.a().write().phase(), TickPhase::Write);
  EXPECT_EQ(cpu.a().read().phase(), TickPhase::Read);
}
```

**Milestone**: Can construct `hdl::Cpu` with all components wired correctly.

**Win**: The HDL tree **exists** and is **type-safe**. Can't create invalid configurations.

---

### Phase 2: Simulator Foundation

**Goal**: Can construct sim tree, tick components, move data via buses.

**Tasks**:
1. `sim::Component` base (with tick phases)
2. `sim::Cpu` (matches HDL, adds state)
3. `sim::ByteBus` (phase-enforced read/write)
4. `sim::Control` (phase-aware value access)
5. `sim::ByteRegister` (value storage + tick behavior)
6. Phase orchestration in `Cpu::Tick()`

**Tests**:
```cpp
TEST(SimTest, RegisterWritesToBus) {
  hdl::Cpu hdl;
  sim::Cpu cpu(hdl);

  cpu.a().SetValue(0x42);
  cpu.a().write().Set(true);
  cpu.Tick();

  EXPECT_THAT(cpu.data_bus(), BusHasValue(0x42));
}

TEST(SimTest, RegisterReadsFromBus) {
  hdl::Cpu hdl;
  sim::Cpu cpu(hdl);

  cpu.a().SetValue(0x42);
  cpu.a().write().Set(true);
  cpu.b().read().Set(true);
  cpu.Tick();

  EXPECT_THAT(cpu.b(), RegisterHasValue(0x42));
}

TEST(SimTest, PhaseViolationDetected) {
  hdl::Cpu hdl;
  sim::Cpu cpu(hdl);

  // Try to read bus during Write phase
  cpu.current_phase_ = TickPhase::Write;
  EXPECT_THROW(cpu.data_bus().Read(), PhaseViolationError);
}
```

**Milestone**: Can tick simulator and move bytes between registers via bus.

**Win**: The five-phase model **works**. Can see hardware simulation in action.

---

### Phase 3: Microcode DSL

**Goal**: Can write instruction behavior in fluent C++ DSL.

**Tasks**:
1. `MicrocodeBuilder` class (fluent interface)
2. Basic operations: `ReadMemoryInto(reg)`, `WriteMemoryFrom(reg)`
3. Higher-level operations: `Move(src, dst)`, `AluOp(opcode)`
4. DSL → IR conversion
5. Define 2-3 simple instructions (LDA, STA, HLT)

**Tests**:
```cpp
TEST(MicrocodeTest, BuildLDA) {
  hdl::Cpu hdl;

  auto ir = MicrocodeBuilder(hdl)
    .ReadMemoryInto(hdl.a())
    .Build();

  EXPECT_EQ(ir.steps.size(), 1);
  EXPECT_TRUE(ir.steps[0].controls.contains(&hdl.memory().read()));
  EXPECT_TRUE(ir.steps[0].controls.contains(&hdl.a().write()));
}
```

**Milestone**: Can express instruction semantics in DSL.

**Win**: Instructions are **readable programs**, not magic.

---

### Phase 4: Microcode Compiler

**Goal**: Multi-pass compilation transforms and validates microcode.

**Tasks**:
1. `Pass` interface
2. `FetchPass` (add fetch stage to all instructions)
3. `SequencePass` (add sequence counter management)
4. `BusValidator` (verify single-writer rule)
5. `StepMerger` (optimize compatible steps)
6. `Compiler` (orchestrate passes)

**Tests**:
```cpp
TEST(CompilerTest, FetchPassAddsStage0) {
  auto ir = BuildSimpleIR();
  ir = FetchPass().Transform(std::move(ir));

  EXPECT_EQ(ir.steps[0].stage, 0);  // Fetch stage
  EXPECT_TRUE(ir.steps[0].controls.contains(&hdl.controller().fetch()));
}

TEST(CompilerTest, BusValidatorDetectsConflict) {
  auto ir = BuildConflictingIR();  // Two writes to same bus

  EXPECT_THROW(
    BusValidator().Transform(std::move(ir)),
    BusConflictError);
}

TEST(CompilerTest, StepMergerCombinesSteps) {
  auto ir = BuildMergeableIR();
  auto orig_size = ir.steps.size();

  ir = StepMerger().Transform(std::move(ir));

  EXPECT_LT(ir.steps.size(), orig_size);
}
```

**Milestone**: Can compile DSL → validated, optimized IR.

**Win**: Bugs caught **before runtime**. Can inspect IR between passes.

---

### Phase 5: Encoding & Execution (THE SPARK!)

**Goal**: Controller reads ROM and makes hardware execute instructions.

**Tasks**:
1. `ControlEncoder` (HDL→Sim mapping, encode/decode)
2. `StatusEncoder` (status flag encoding)
3. `InstructionMemory` (ROM storage)
4. `Controller` component (fetch-decode-execute)
5. Integration: compile microcode → load ROM → execute

**Tests**:
```cpp
TEST(EncoderTest, MapsHdlToSim) {
  hdl::Cpu hdl;
  sim::Cpu cpu(hdl);

  ControlEncoder encoder(hdl, cpu);

  // Verify mapping works
  auto word = encoder.Encode({&hdl.a().write()});
  auto controls = encoder.Decode(word);

  EXPECT_EQ(controls.size(), 1);
  EXPECT_EQ(controls[0], &cpu.a().write());
}

TEST(ControllerTest, FetchesOpcode) {
  auto harness = CpuTestHarness()
    .WithProgram({0xA0, 0x42});  // LDA #$42

  harness.cpu().Tick();  // Fetch

  EXPECT_THAT(harness.cpu().controller().ir(), RegisterHasValue(0xA0));
}

TEST(ExecutionTest, LDA_Immediate) {
  auto harness = CpuTestHarness()
    .WithProgram({0xA0, 0x42, 0x01});  // LDA #$42, HLT

  harness.RunUntilHalt();

  EXPECT_THAT(harness.cpu().a(), RegisterHasValue(0x42));
}
```

**Milestone**: Can execute LDA and HLT instructions!

**Win**: THE SPARK WORKS. Microcode brings hardware to life. 🎉

---

### Phase 6: Minimum Viable Program (HLT)

**Goal**: Can run a program that just halts.

**Program**:
```asm
HLT  ; Opcode 0x01
```

**Components Required**:
- ✅ Cpu
- ✅ Controller (fetch, decode, execute)
- ✅ InstructionRegister
- ✅ SequenceCounter
- ✅ InstructionMemory (ROM)
- ✅ ProgramCounter
- ✅ Memory (to read HLT opcode)
- ✅ Address bus
- ✅ Data bus

**HLT Microcode**:
```cpp
ir::Instruction HLT(const hdl::Cpu& hdl) {
  return MicrocodeBuilder(hdl)
    .SetHaltFlag()  // ProcessControl that sets cpu.halted = true
    .Build();
}
```

**Test**:
```cpp
TEST(MinimalProgram, HLT) {
  auto harness = CpuTestHarness()
    .WithProgram({0x01});  // HLT

  harness.RunUntilHalt();

  EXPECT_TRUE(harness.cpu().halted());
  EXPECT_EQ(harness.cpu().cycle_count(), 1);  // Just fetch+execute
}
```

**Milestone**: First complete program runs!

**Win**: End-to-end system works. Have proven the architecture.

---

### Phase 7: Core Instructions (Vertical Slice)

**Goal**: Implement enough instructions to write interesting programs.

**Instructions** (minimal set):
- LDA, LDX (load registers)
- STA, STX (store to memory)
- ADC (add with carry)
- CMP (compare)
- JMP (unconditional jump)
- BEQ, BNE (conditional branch)
- HLT (halt)

**New Components**:
- ALU (with Add, Compare operations)
- StatusRegister (zero, negative, carry flags)

**Test Program**:
```asm
; Add 3 + 4 = 7
LDA #$03
ADC #$04
CMP #$07
BNE crash
HLT

crash:
  CRS  ; Crash opcode
```

**Milestone**: Can write simple programs with arithmetic and control flow.

**Win**: System feels **real**. Can express algorithms.

---

### Phase 8: ISA Module & Assembler

**Goal**: Define ISA in YAML, generate C++/Python code, assemble .asm files.

**Tasks**:
1. Define `instructions.yaml` (opcodes, addressing modes)
2. Code generator: YAML → C++ enums/tables
3. Code generator: YAML → Python bindings
4. Python assembler (lexer, parser, assembler)
5. Integration: .asm → .bin → simulator

**Tests**:
```python
def test_assemble_lda():
    source = "LDA #$42"
    binary = assemble(source)
    assert binary == bytes([0xA0, 0x42])

def test_assemble_program():
    source = """
        LDA #$03
        ADC #$04
        HLT
    """
    binary = assemble(source)
    # Run in simulator, verify result
```

**Milestone**: Can write programs in assembly syntax.

**Win**: Human-readable programs, not hex dumps.

---

### Phase 9: Full ISA (IRATA Instructions)

**Goal**: Implement complete instruction set from IRATA/PIRATA.

**Instructions** (~50 total):
- Loads: LDA, LDX, LDY (all addressing modes)
- Stores: STA, STX, STY
- Transfers: TAX, TAY, TXA, TYA, TSX, TXS
- Stack: PHA, PLA, PHP, PLP
- Arithmetic: ADC, SBC
- Logic: AND, ORA, EOR
- Shifts: ASL, LSR, ROL, ROR
- Comparisons: CMP, CPX, CPY
- Branches: BEQ, BNE, BCS, BCC, BMI, BPL, BVS, BVC
- Jumps: JMP, JSR, RTS, RTI
- System: BRK, NOP, HLT, CRS

**Test Programs**: Port from PIRATA
- Fibonacci
- Stack operations
- Interrupt handling
- Edge cases

**Milestone**: Feature-complete instruction set.

**Win**: Can run complex programs. PIRATA's test suite passes.

---

### Phase 10: MMIO & Device System

**Goal**: Memory-mapped I/O for cartridges and devices.

**Components**:
- `MemoryRegion` interface
- `RomCartridge` (read-only memory)
- `RamRegion` (read-write memory)
- `ConsoleDevice` (character output)
- `KeyboardDevice` (input)

**Memory Map**:
```
0x0000-0x00FF: Zero page (stack)
0x0100-0x3FFF: RAM
0x6000-0x6001: Console I/O
0x6002-0x6003: Keyboard
0x8000-0xFFFF: ROM (cartridge)
```

**Test Program**:
```asm
; Echo input to output
loop:
  LDA $6002    ; Read keyboard
  BEQ loop     ; Wait for input
  STA $6000    ; Write to console
  JMP loop
```

**Milestone**: Can interact with I/O devices.

**Win**: System is **interactive**. Feels like real hardware.

---

### Phase 11: The Dream - Simple Graphics

**Goal**: Memory-mapped PPU for vector graphics.

**Design**:
- PPU device at 0x7000-0x7FFF
- Command buffer in memory
- Simple vector language: MOVE(x,y), LINE(x,y), CLEAR
- Pygame renderer reads commands and draws

**Memory Layout**:
```
0x7000: Command count
0x7001: Command buffer
  - Byte 0: Command (MOVE=1, LINE=2, CLEAR=3)
  - Byte 1-2: X coordinate (word)
  - Byte 3-4: Y coordinate (word)
```

**Test Program** (draw triangle):
```asm
; Clear screen
LDA #$03
STA $7001

; Move to (100, 100)
LDA #$01
STA $7001
LDA #$64
STA $7002
STA $7004

; Line to (200, 100)
LDA #$02
STA $7001
LDA #$C8
STA $7002
LDA #$64
STA $7004

; Line to (150, 200)
LDA #$02
STA $7001
LDA #$96
STA $7002
LDA #$C8
STA $7004

HLT
```

**Milestone**: Can draw vector graphics!

---

### Phase 12: Asteroids (The Dream)

**Goal**: Playable vector graphics Asteroids game.

**Features**:
- Ship with rotation and thrust
- Shooting mechanics
- Asteroid spawning and collision
- Score tracking
- Game over logic

**All written in assembly**. Running on the simulator. With keyboard input. Rendering to Pygame.

**Milestone**: The dream is real. 🚀

**Win**: Can proudly show people. They'll say "that's a pygame tutorial." You'll laugh hilariously. Because you built a CPU from scratch.

---

## Design Decisions (Resolved)

### 1. Control Type System & Phase Access

**Controls are fundamental components with strong type system representation.**

**Key Properties**:
- **Writable**: Only during `TickControl()` phase (microcode assertion)
- **Readable**: During configurable phase specific to control type
  - `WriteControl` → readable during `TickWrite()`
  - `ReadControl` → readable during `TickRead()`
  - `ProcessControl` → readable during `TickProcess()`
- **Bus Connection**: Read/Write controls must represent their connected bus (critical for bus validation in microcode compiler)
- **Reset Behavior**: Two types
  - **Auto-reset**: Value cleared during `TickClear()` (most controls)
  - **Latched**: Persists across ticks until manually reset

**Latched Control Pattern**:
```cpp
class LatchedControl {
  LatchedControl(Component& parent, const std::string& name, TickPhase phase)
    : value_control_(name, parent, phase, /*auto_clear=*/false),
      reset_control_(name + ".reset", parent, TickPhase::Process, /*auto_clear=*/true) {}

  const Control& value() const { return value_control_; }
  const ProcessControl& reset() const { return reset_control_; }  // Microcode can assert reset

private:
  Control value_control_;
  UnlatchedProcessControl reset_control_;
};
```

**Type Hierarchy**:
```cpp
class Control : public ComponentWithParent {
public:
  Control(Component& parent, const std::string& name,
          TickPhase readable_phase, bool auto_clear)
    : ComponentWithParent(parent),
      name_(name),
      readable_phase_(readable_phase),
      auto_clear_(auto_clear) {}

  // Only writable during Control phase
  void Set(bool value) {
    if (cpu().current_phase() != TickPhase::Control) {
      throw PhaseViolationError("Controls only writable during Control phase");
    }
    value_ = value;
  }

  // Only readable during assigned phase
  bool value() const {
    if (cpu().current_phase() != readable_phase_ &&
        cpu().current_phase() != TickPhase::None) {
      throw PhaseViolationError(
        std::format("Control {} (phase {}) read during phase {}",
                    path(), ToString(readable_phase_),
                    ToString(cpu().current_phase())));
    }
    return value_;
  }

  void TickClear() override {
    if (auto_clear_) {
      value_ = false;
    }
  }

private:
  std::string name_;
  TickPhase readable_phase_;
  bool auto_clear_;
  bool value_ = false;
};

// Specialized control types
class WriteControl : public Control {
public:
  WriteControl(Component& parent, const std::string& name, Bus& bus)
    : Control(parent, name, TickPhase::Write, /*auto_clear=*/true),
      bus_(bus) {}

  const Bus& bus() const { return bus_; }  // For bus validation

private:
  Bus& bus_;
};

class ReadControl : public Control {
public:
  ReadControl(Component& parent, const std::string& name, Bus& bus)
    : Control(parent, name, TickPhase::Read, /*auto_clear=*/true),
      bus_(bus) {}

  const Bus& bus() const { return bus_; }

private:
  Bus& bus_;
};

class ProcessControl : public Control {
public:
  ProcessControl(Component& parent, const std::string& name, bool auto_clear = true)
    : Control(parent, name, TickPhase::Process, auto_clear) {}
};
```

**Why This Matters**:
Strong control typing enables:
1. Microcode compiler can validate bus conflicts (check `control.bus()`)
2. Optimizer can reason about control interactions (different phases → can merge)
3. Hardware-ish behavior (latched vs auto-reset matches real control logic)
4. Clear, enforceable phase rules

### 2. HDL Validation Against Simulator

**Yes, validate structure during `sim::Cpu` construction.**

```cpp
namespace sim {
  class Cpu : public Component {
    static std::unique_ptr<Cpu> Create(const hdl::Cpu& hdl) {
      auto cpu = std::make_unique<Cpu>(hdl);
      ValidateStructure(hdl, *cpu);  // Assert matching structure
      return cpu;
    }

  private:
    static void ValidateStructure(const hdl::Cpu& hdl, const sim::Cpu& sim) {
      // Validate component tree matches
      assert(hdl.a().name() == sim.a().name());
      assert(&hdl.a().bus() == &hdl.data_bus());  // HDL structure
      assert(&sim.a().bus() == &sim.data_bus());  // Sim structure
      // ... validate all components
    }
  };
}
```

This catches bugs where HDL and sim diverge.

### 3. Component Paths for Debugging

**Yes, implement paths for debugging and potentially for instruction encoding.**

```cpp
class Component {
  virtual std::string path() const = 0;
};

class Cpu : public Component {
  std::string path() const override { return "/cpu"; }
};

class ComponentWithParent : public Component {
  std::string path() const override {
    return parent_.path() + "/" + name_;
  }

private:
  std::string name_;
};
```

Paths used for:
- Error messages: `"Bus /cpu/data_bus has conflict"`
- Debug output: `"Asserting control /cpu/a/write"`
- Possibly control mapping during instruction encoding (open question whether this is cleaner than parallel traversal)

### 4. Memory Architecture (Detailed Specification)

**Memory is a bus-connected component with modular, hardware-ish structure.**

```cpp
class Memory : public ComponentWithParent {
public:
  Memory(Component& parent, ByteBus& data_bus, WordBus& address_bus)
    : ComponentWithParent(parent),
      mar_("mar", *this, address_bus, data_bus),  // MAR is a WordRegister
      read_control_("read", *this, data_bus),
      write_control_("write", *this, data_bus) {
    // Regions own modules
    regions_.push_back(std::make_unique<Region>(
      0x0000, 0x4000, std::make_unique<RamModule>(0x4000)));  // 16KB RAM
    regions_.push_back(std::make_unique<Region>(
      0x8000, 0x8000, std::make_unique<RomModule>(rom_data)));  // 32KB ROM
  }

  void TickRead() override {
    if (read_control_.value()) {
      uint16_t addr = mar_.value();
      uint8_t data = ReadFromRegions(addr);
      data_bus_.Write(Byte(data));
    }
  }

  void TickWrite() override {
    if (write_control_.value()) {
      uint16_t addr = mar_.value();
      uint8_t data = data_bus_.Read().value;
      WriteToRegions(addr, data);
    }
  }

  const WordRegister& mar() const { return mar_; }

private:
  WordRegister mar_;  // Memory Address Register (connected to address bus)
  ReadControl read_control_;
  WriteControl write_control_;
  ByteBus& data_bus_;
  std::vector<std::unique_ptr<Region>> regions_;
};

// WordRegister composed of two ByteRegisters
class WordRegister : public ComponentWithParent {
public:
  WordRegister(Component& parent, const std::string& name,
               WordBus& word_bus, ByteBus& byte_bus)
    : ComponentWithParent(parent),
      high_("high", *this, byte_bus),  // High byte connected to data bus
      low_("low", *this, byte_bus),    // Low byte connected to data bus
      word_read_("word_read", *this, word_bus),
      word_write_("word_write", *this, word_bus) {}

  const ByteRegister& high() const { return high_; }
  const ByteRegister& low() const { return low_; }

  uint16_t value() const {
    return (static_cast<uint16_t>(high_.value()) << 8) | low_.value();
  }

private:
  ByteRegister high_;
  ByteRegister low_;
  ReadControl word_read_;
  WriteControl word_write_;
};

// Module = abstract contiguous power-of-2 memory chunk
class Module {
public:
  virtual ~Module() = default;
  virtual uint8_t Read(uint16_t offset) const = 0;
  virtual void Write(uint16_t offset, uint8_t value) = 0;
  virtual uint16_t size() const = 0;  // Must be power of 2
};

class RamModule : public Module {
public:
  explicit RamModule(uint16_t size) : data_(size, 0x00) {
    assert((size & (size - 1)) == 0);  // Power of 2
  }

  uint8_t Read(uint16_t offset) const override { return data_[offset]; }
  void Write(uint16_t offset, uint8_t value) override { data_[offset] = value; }
  uint16_t size() const override { return data_.size(); }

private:
  std::vector<uint8_t> data_;
};

class RomModule : public Module {
public:
  explicit RomModule(std::vector<uint8_t> data) : data_(std::move(data)) {
    assert((data_.size() & (data_.size() - 1)) == 0);  // Power of 2
  }

  uint8_t Read(uint16_t offset) const override { return data_[offset]; }
  void Write(uint16_t offset, uint8_t value) override {
    // ROM ignores writes (hardware-ish)
  }
  uint16_t size() const override { return data_.size(); }

private:
  std::vector<uint8_t> data_;
};

// Region maps memory space to module address space
class Region {
public:
  Region(uint16_t base, uint16_t size, std::unique_ptr<Module> module)
    : base_(base), size_(size), module_(std::move(module)) {
    assert(module_->size() == size);
  }

  bool Contains(uint16_t addr) const {
    return addr >= base_ && addr < base_ + size_;
  }

  uint8_t Read(uint16_t addr) const {
    return module_->Read(addr - base_);
  }

  void Write(uint16_t addr, uint8_t value) {
    module_->Write(addr - base_, value);
  }

private:
  uint16_t base_;
  uint16_t size_;
  std::unique_ptr<Module> module_;
};
```

**Hardware-ish**: Memory owns MAR, MAR has byte access to data bus for loading addresses, regions map to modules, modules are power-of-2 chunks like real memory ICs.

### 5. Status Register & Instruction ROM Encoding

**Hardware-ish instruction ROM with "don't care" status encoding.**

```cpp
class StatusRegister : public ComponentWithParent {
public:
  const Status& zero() const { return zero_; }
  const Status& negative() const { return negative_; }
  const Status& carry() const { return carry_; }
  const Status& overflow() const { return overflow_; }

  // Encode all status lines into bit pattern
  uint8_t encode() const {
    return (zero_.value() << 0) |
           (negative_.value() << 1) |
           (carry_.value() << 2) |
           (overflow_.value() << 3);
  }

private:
  Status zero_;
  Status negative_;
  Status carry_;
  Status overflow_;
};

class Status : public ComponentWithParent {
public:
  bool value() const {
    // Readable during Control (for conditionals) and Process (for updates)
    if (cpu().current_phase() != TickPhase::Control &&
        cpu().current_phase() != TickPhase::Process &&
        cpu().current_phase() != TickPhase::None) {
      throw PhaseViolationError("Status readable during Control and Process only");
    }
    return value_;
  }

  void Set(bool v) {
    if (cpu().current_phase() != TickPhase::Process) {
      throw PhaseViolationError("Status writable during Process only");
    }
    value_ = v;
  }

private:
  bool value_ = false;
};
```

**Instruction ROM Encoding** (hardware-ish):

When burning ROM, encode **every useful instruction address**:

```cpp
class InstructionMemoryBurner {
  void BurnInstruction(const ir::Instruction& instruction) {
    for (const auto& step : instruction.steps) {
      if (step.status_conditions.empty()) {
        // "Don't care" - burn all possible status combinations
        for (uint8_t status = 0; status < 16; ++status) {  // 4 status bits = 16 combos
          InstructionAddress addr{instruction.opcode, step.stage, status};
          rom_[addr] = EncodeControlWord(step.controls);
        }
      } else {
        // Specific status conditions - burn only matching combinations
        for (uint8_t status = 0; status < 16; ++status) {
          if (StatusMatches(status, step.status_conditions)) {
            InstructionAddress addr{instruction.opcode, step.stage, status};
            rom_[addr] = EncodeControlWord(step.controls);
          }
        }
      }
    }
  }
};
```

**Controller Lookup** (no magic):

```cpp
void Controller::TickControl() {
  // Opcode + step + ALL status lines
  InstructionAddress addr{
    .opcode = ir_.value(),
    .step = sequence_counter_.value(),
    .status = cpu().status().encode()  // All 4 status bits
  };

  // Just look up. Hardware-ish.
  auto control_word = instruction_memory_.Read(addr);

  // Decode and assert
  auto controls = control_encoder_.Decode(control_word.controls);
  for (auto* ctrl : controls) {
    ctrl->Set(true);
  }
}
```

**Bonus Hardware-ish Idea**: Implement InstructionMemory using the ROM module!

```cpp
class InstructionMemory {
  InstructionMemory() {
    // Multiple 8-bit ROMs mapped together (like real hardware)
    // Address = (opcode << 12) | (step << 8) | (status << 0)
    // Data = 64-bit control word spread across 8 ROM modules
    for (int i = 0; i < 8; ++i) {
      rom_modules_[i] = std::make_unique<RomModule>(/* 64KB each */);
    }
  }

  ControlWord Read(InstructionAddress addr) const {
    uint16_t rom_addr = (addr.opcode << 12) | (addr.step << 8) | addr.status;
    uint64_t word = 0;
    for (int i = 0; i < 8; ++i) {
      word |= static_cast<uint64_t>(rom_modules_[i]->Read(rom_addr)) << (i * 8);
    }
    return ControlWord{word};
  }

private:
  std::array<std::unique_ptr<RomModule>, 8> rom_modules_;  // 8 x 8-bit ROMs
};
```

Burning multiple 8-bit ROMs together to build a wide control word. **Hardware-ish!**

### 6. Assembler Architecture

**Separate tool, common ISA definition, outputs binary + debug symbols.**

```
┌─────────────────────────────────────────┐
│         instructions.yaml               │
│  (Single source of truth for ISA)       │
└──────────────┬──────────────────────────┘
               │
        ┌──────┴──────┐
        │             │
        ▼             ▼
   ┌─────────┐   ┌─────────┐
   │C++ Gen  │   │Python   │
   │         │   │Gen      │
   └────┬────┘   └────┬────┘
        │             │
        ▼             ▼
   ┌─────────┐   ┌─────────┐
   │asm/*.h  │   │asm/*.py │
   │(enums)  │   │(enums)  │
   └─────────┘   └─────────┘
        │             │
        └──────┬──────┘
               │
        ┌──────┴──────────┐
        │                 │
        ▼                 ▼
   ┌─────────┐      ┌──────────┐
   │Sim uses │      │Assembler │
   │C++ ISA  │      │uses Py   │
   └─────────┘      │ISA       │
                    └─────┬────┘
                          │
                          ▼
                    ┌──────────┐
                    │program.  │
                    │bin       │
                    │+debug.   │
                    │json      │
                    └──────────┘
```

**Language**: Python or C++ (both acceptable)
**Input**: `.asm` files
**Output**:
- `program.bin` (binary machine code)
- `program.debug.json` (optional symbol table for debugging)

**Integration**: Assembler is invoked as subprocess during tests:
```cpp
TEST_P(AsmProgramTest, Execute) {
  auto asm_file = GetParam();

  // Invoke assembler
  auto result = std::system(std::format("./assembler {} -o {}.bin",
                                        asm_file, asm_file).c_str());
  ASSERT_EQ(result, 0);

  // Load and run
  auto binary = LoadBinary(std::format("{}.bin", asm_file));
  auto harness = CpuTestHarness().WithProgram(binary);
  harness.RunUntilHalt();

  EXPECT_TRUE(harness.cpu().halted());
}
```

### 7. Performance Target

**Goal: 2000 instructions/second (2x PIRATA's 1000 instructions/sec)**

PIRATA (pure Python) achieved ~1000 instructions/sec. C++ should easily hit 2x.

**Optimization Strategy**:
1. **Phase 0-6**: Don't optimize. Focus on correctness.
2. **Phase 7**: First performance measurement. Profile.
3. **Phase 8+**: Optimize hot paths if needed.
4. **Target**: 2000 inst/sec is table stakes. Likely we'll exceed this significantly.

**Measurement**:
```cpp
TEST(PerformanceTest, InstructionsPerSecond) {
  auto harness = CpuTestHarness().WithProgram(LoadFibonacci());

  auto start = std::chrono::high_resolution_clock::now();
  harness.RunUntilHalt();
  auto end = std::chrono::high_resolution_clock::now();

  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
  auto inst_per_sec = (harness.cpu().instruction_count() * 1000) / duration.count();

  EXPECT_GT(inst_per_sec, 2000);  // Must exceed target
}
```

**Why 2000 is enough**: For 60fps interactive game, budget is ~33 instructions per frame. Very achievable.

---

## Technology Stack

- **Language**: C++20 (Clang 15+)
- **Build**: CMake 3.20+
- **Testing**: GoogleTest + GoogleMock
- **Coverage**: lcov (100% required)
- **Format**: clang-format
- **Lint**: clang-tidy
- **Python**: Python 3.11+ (assembler)
- **YAML**: yaml-cpp (ISA definitions)
- **Graphics**: Pygame (for visualization)

---

## Success Criteria

**Foundation** (Phases 0-2):
- ✅ Can write beautiful, readable tests
- ✅ Can construct immutable HDL tree
- ✅ Can tick simulator and move data via buses

**Core System** (Phases 3-6):
- ✅ Can write microcode in DSL
- ✅ Can compile and validate microcode
- ✅ Can execute instructions (THE SPARK)
- ✅ Can run minimal program (HLT)

**Functional CPU** (Phases 7-9):
- ✅ Can write programs with arithmetic and control flow
- ✅ Can assemble .asm files
- ✅ Can run full IRATA instruction set
- ✅ 100% test coverage

**Interactive System** (Phases 10-11):
- ✅ Can load cartridges
- ✅ Can interact with MMIO devices
- ✅ Can draw vector graphics

**The Dream** (Phase 12):
- ✅ Can play Asteroids
- ✅ Can laugh hilariously at skeptics

---

## Philosophy Recap

This is a **learning project** about deeply understanding CPUs through implementation.

**Hardware-ish**: Pretend we're building with breadboards and wires, but in type-safe C++.

**Strongly typed**: Invalid states are unrepresentable. The type system is our friend.

**Testable**: 100% coverage, tests as documentation, infrastructure makes tests readable.

**Incremental**: Frequent wins, vertical slices, always working.

**The spark**: Microcode (brain) + hardware (brawn) = magic.

**The dream**: Vector graphics Asteroids running on a CPU built from scratch.

Let's build something beautiful. 🚀
