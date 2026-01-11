# Sim Module Review

**Status:** Production-ready
**Files:** 61 (31 headers, 16 source, 14 test)
**Issues:** 2 low-priority

## Summary

Runtime simulator implementing the five-phase tick model with phase-aware controls, single-writer buses, and controller-driven microcode execution.

## Design Alignment

| Requirement | Status | Evidence |
|-------------|--------|----------|
| Five-phase tick model | PASS | Control → Write → Read → Process → Clear |
| Phase-aware controls | PASS | Compile-time + runtime enforcement |
| Single-writer buses | PASS | wrote_this_tick_ flag |
| Hardware-ish components | PASS | Components don't know instruction semantics |
| Structural match validation | PASS | At LoadProgram time |

## Five-Phase Implementation

```cpp
void Cpu::Tick() {
  current_phase_ = TickPhase::Control;  TickPhase(&Component::TickControl);
  current_phase_ = TickPhase::Write;    TickPhase(&Component::TickWrite);
  current_phase_ = TickPhase::Read;     TickPhase(&Component::TickRead);
  current_phase_ = TickPhase::Process;  TickPhase(&Component::TickProcess);
  current_phase_ = TickPhase::Clear;    TickPhase(&Component::TickClear);
}
```

## Bus Single-Writer Enforcement

```cpp
void Write(ValueType value, std::string_view writer_path) {
  if (current_phase() != base::TickPhase::Write)
    throw SimError("bus write outside write phase");
  if (wrote_this_tick_)
    throw SimError("bus already written");
  // ...
}
```

## Virtual Functions

Virtual functions ARE used in sim (unlike HDL) - this is acceptable:
- Enables runtime polymorphism for component hierarchy
- Type erasure needed for control index building
- Not a performance bottleneck (one tick per cycle)

## Findings

### L1: Unmapped Read Magic Number
```cpp
if (!region) return base::Byte{0xFF};  // Should document why
```

### L2: Single dynamic_cast
```cpp
if (auto* control = dynamic_cast<ControlBase*>(component))
```
Necessary for type filtering; could add comment.
