# HDL Module

Hardware Definition Language - immutable structural metadata representing the CPU architecture.

## Overview

The HDL module defines the **structure** of the CPU without runtime state. It's like a schematic: it describes what components exist, how they're connected, and what controls they expose. The simulator (sim module) adds the actual runtime behavior.

## Design Principles

- **Immutable**: Once constructed, the component tree cannot be modified
- **Compile-time safety**: CRTP templates ensure type safety with zero runtime overhead
- **Strongly typed**: Component relationships are enforced at compile time
- **Inspectable**: The entire structure can be visited and enumerated

## Component Hierarchy

```
Component<Derived> (CRTP base)
└── ComponentWithParent<Derived>
    ├── Bus<ValueType>
    │   ├── ByteBus
    │   └── WordBus
    ├── ComponentWithBus<Derived, ValueType>
    ├── Control<Derived, ValueType, Phase, AutoReset>
    │   ├── WriteControl<ValueType>
    │   ├── ReadControl<ValueType>
    │   └── ProcessControl<AutoReset>
    ├── Register<Derived, ValueType>
    │   ├── ByteRegister
    │   ├── WordRegister
    │   └── Counter<ValueType>
    ├── LocalRegister<Derived, ValueType>  (no bus)
    │   └── LocalCounter<ValueType>
    │       └── ByteCounter
    └── Controller
```

## Key Concepts

### CRTP (Curiously Recurring Template Pattern)

All components use CRTP for zero-cost static dispatch. Each derived type provides
`visit_impl()` so the base can call into the most-derived type without virtuals.
Parents are type-erased to maximize reuse; `cpu()` and `path()` are stored in a
non-templated base to avoid virtuals.

```cpp
template<typename Derived>
class Component : public ComponentBase {
public:
  Derived& self() { return static_cast<Derived&>(*this); }
  const Derived& self() const { return static_cast<const Derived&>(*this); }

  using ComponentBase::ComponentBase;

  template<typename Visitor>
  void visit(Visitor&& visitor) const {
    self().visit_impl(std::forward<Visitor>(visitor));
  }
};
```

### Buses

Buses are typed by the value they transport:

```cpp
template<typename ValueType>
class Bus : public ComponentWithParent<Bus<ValueType>> {
  using value_type = ValueType;
  static constexpr size_t kWidth = sizeof(ValueType) * 8;
};

using ByteBus = Bus<base::Byte>;
using WordBus = Bus<base::Word>;
```

### Controls

Controls specify their behavior at compile time:

```cpp
template<typename ValueType>
class WriteControl
  : public Control<WriteControl<ValueType>, ValueType,
                   TickPhase::Write, /*AutoReset=*/true> {
  const auto& bus() const;  // For bus conflict validation
};
```

Control template parameters:
- `Phase`: Which tick phase this control operates in
- `AutoReset`: Whether the control auto-clears after each tick

Each control stores a `ControlInfo` with its precomputed properties
(`phase`, `auto_reset`, and stable `path` string).

### Registers

Components that participate in bus transfers share a common base:

```cpp
template<typename Derived, typename ValueType>
class ComponentWithBus : public ComponentWithParent<Derived> {
  WriteControl<ValueType> write_control_;
  ReadControl<ValueType> read_control_;
  const Bus<ValueType>& bus_;
};
```

Registers are one example of `ComponentWithBus` and include a reset control:

```cpp
template<typename Derived, typename ValueType>
class Register : public ComponentWithBus<Derived, ValueType> {
  ProcessControl<true> reset_control_;
};
```

### CPU Structure

The `Cpu` class is the only entry point for creating HDL:

```cpp
class Cpu final : public Component<Cpu> {
  ByteBus data_bus_;
  WordBus address_bus_;
  ByteRegister a_;               // Accumulator
  Counter<base::Word> pc_;       // Program counter
  Controller controller_;        // IR + SC
  ByteRegister status_;          // SR (bus-connected)
  StatusZero zero_;
  StatusNegative negative_;
  StatusCarry carry_;
  StatusOverflow overflow_;
  StatusInterruptDisable interrupt_disable_;
  StatusDecimal decimal_;
  StatusBreak break_;
  StatusUnused unused_;
  ProcessControl<true> halt_;     // Halt execution
  ProcessControl<true> crash_;    // Crash execution
  // ...
};
```

### Status Register (SR)

The status register is bus-connected in HDL and exposes named status signals as
views into specific bits. Push/pop wiring is planned in the simulator. The
6502-style bit layout is used:

```
bit:  7   6   5   4   3   2   1   0
name: N   V   U   B   D   I   Z   C
```

- `status` is the register name in HDL.
- Status components are named with full words: `zero`, `negative`, `carry`,
  `overflow`, `interrupt_disable`, `decimal`, `break`, `unused`.
- Default SR value is `0x00`.

## Structural Inspection

Components support visitation for enumeration:

```cpp
struct ControlCounter {
  size_t count = 0;

  template<typename C>
  void operator()(const C& component) {
    if constexpr (is_control_v<C>) {
      ++count;
    }
  }
};

Cpu cpu;
ControlCounter counter;
cpu.visit(counter);
// counter.count has total control count
```

Path resolution for microcode is handled by the microcode module:

```cpp
irata2::microcode::ir::CpuPathResolver resolver(GetCpu());
const auto* control = resolver.RequireControl("controller.ir.write", "example");
```

## Detailed Implementation Plan

### 1) Core CRTP and component base
- Add `component_base.h` for `name()`, `path()`, and `cpu()` storage.
- Add `component.h` with `Component<Derived>` and `ComponentWithParent<Derived>`.
- Store `name_` in each component; compute `path()` once at construction.
- Add `visit_impl()` pattern that visits self then children (implemented per type).

### 2) Traits and helper utilities
- `traits.h`: `is_control_v`, `is_bus_v`, `is_register_v`, `is_status_v`.
- Provide a small `kind()` enum or trait for diagnostics and tests.
- `path()` and `name()` helpers with consistent separators.

### 3) Leaf and structural components
- `bus.h`: `Bus<ValueType>` with width and value type aliases.
- `control.h`: `Control<Derived, ValueType, Phase, AutoReset>` with `phase()` and `auto_reset()`.
- `component_with_bus.h`: `ComponentWithBus<Derived, ValueType>` for read/write bus participants.
- `register.h`: `Register`, `WordRegister`, `Counter` with const members and typed accessors.

### 4) CPU composition
- `cpu.h`: assemble const members in strict construction order.
- Provide strongly typed accessors (`cpu().memory().mar()` pattern).
- Implement `visit_impl()` by calling `visit()` on child members.
- Add `MapControls(sim::Cpu&, ControlEncoder&)` stubs as compile-time hooks.

### 5) Tests
- Extend HDL tests to cover `path()`, `name()`, `cpu()` on nested components.
- Add traversal tests that count controls/buses/registers via `visit()`.
- Add compile-time static assertions for trait correctness.

### 6) Integration readiness
- Ensure headers are installed by CMake (already in place).
- Keep runtime-free: no mutable state, no sim dependencies in HDL.

## Design Clarifications / Corrections

- Parents are type-erased to maximize reuse; components should be freely
  composable under many different owners.
- CRTP base should not rely on virtuals; `visit_impl()` is required on derived
  types for static dispatch.

## Usage

```cmake
target_link_libraries(your_target PRIVATE irata2::hdl)
```

```cpp
#include "irata2/hdl.h"

irata2::hdl::Cpu cpu;
// Access components via strongly-typed accessors
const auto& data_bus = cpu.data_bus();
const auto& a_reg = cpu.a();
```

## Files

- `component.h` - CRTP base classes
- `traits.h` - Type traits for CRTP
- `bus.h` - Bus<T> template
- `control.h` - Control hierarchy
- `register.h` - Register hierarchy
- `cpu.h` - IRATA CPU structure
