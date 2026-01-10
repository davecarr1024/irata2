# IRATA2 Code Review Report

**Date**: 2026-01-10
**Scope**: Full codebase review against stated design goals
**Reviewer**: Automated code review

---

## Executive Summary

The IRATA2 codebase demonstrates **strong adherence to most design principles** with a solid foundation for a hardware-ish CPU simulator. The most critical architectural violations around control metadata and HDL immutability have been resolved; remaining issues are primarily in simulator phase enforcement and sim-side path caches.

### Summary Metrics

| Category | Status |
|----------|--------|
| Major Issues | 2 (Issues 3, 5) |
| Minor Issues | 8 |
| Design Compliance | ~90% |
| Test Coverage | 88.6% lines, 98.0% functions (filtered) |
| Module Separation | Excellent |

---

## Design Goal Compliance Matrix

| Design Goal | Status | Notes |
|-------------|--------|-------|
| Hardware-ish design | PARTIAL | Controls lack phase enforcement in sim |
| CRTP for zero-cost dispatch | PASS | ControlInfo replaces ControlBase virtuals |
| Immutable HDL structure | PASS | Mutable caches removed from HDL Cpu |
| Strongly typed navigation | PASS | Path resolution moved to microcode |
| Type-safe Byte/Word | PASS | Properly used throughout |
| Five-phase tick model | PARTIAL | Phase validation missing in sim controls |
| 100% test coverage | PARTIAL | 88.6% lines, 98.0% functions (filtered) |
| Clean module boundaries | PASS | Excellent separation of concerns |
| No instruction logic in components | PASS | Components are dumb hardware |

---

## Major Issues

### Issue 1: ControlBase Uses Virtual Dispatch Instead of CRTP

**Location**: [hdl/include/irata2/hdl/control.h](hdl/include/irata2/hdl/control.h), [hdl/include/irata2/hdl/control_info.h](hdl/include/irata2/hdl/control_info.h)
**Severity**: HIGH
**Status**: RESOLVED
**Design Violation**: "All HDL components use CRTP for zero-cost static dispatch" (design.md)

**Historical implementation (fixed)**:
```cpp
class ControlBase {
public:
  virtual ~ControlBase() = default;
  virtual base::TickPhase phase() const = 0;
  virtual bool auto_reset() const = 0;
  virtual const std::string& name() const = 0;
  virtual const std::string& path() const = 0;
};
```

The ControlBase class introduces vtable overhead and virtual dispatch where the design explicitly requires CRTP-based zero-cost abstractions. This affects:
- All control signal lookups during encoding
- Any HDL traversal using `ControlBase*`
- Performance of the encoding/decoding path

**Impact**: Breaks the zero-cost abstraction guarantee for control signals.

**Resolution**: ControlBase was removed, ControlInfo now stores non-virtual metadata,
and control properties are accessed via CRTP and ControlInfo.

---

### Issue 2: Mutable Caches in "Immutable" HDL

**Location**: [hdl/include/irata2/hdl/cpu.h](hdl/include/irata2/hdl/cpu.h)
**Severity**: HIGH
**Status**: RESOLVED
**Design Violation**: "The tree structure is immutable after construction" (design.md)

**Historical implementation (fixed)**:
```cpp
// In hdl::Cpu class
mutable bool controls_indexed_ = false;
mutable std::unordered_map<std::string, const ControlBase*> controls_by_path_;
mutable std::vector<std::string> control_paths_;
```

These mutable members allow lazy initialization during const method calls (`ResolveControl`, `AllControlPaths`), violating the immutability guarantee. This creates:
- Hidden side effects in const operations
- Thread-safety concerns
- Unexpected state changes during debugging

**Impact**: HDL is not truly immutable; const methods have side effects.

**Resolution**: Mutable caches and path resolution were removed from HDL Cpu.
Path resolution now lives in `microcode::ir::CpuPathResolver`.

---

### Issue 3: Missing Phase Validation on Control Reads

**Location**: [sim/include/irata2/sim/control.h](sim/include/irata2/sim/control.h)
**Severity**: MEDIUM
**Design Violation**: "Controls can only be read during their assigned phase" (design.md:425)

**Historical implementation (fixed)**:
```cpp
class Control {
public:
  bool asserted() const { return asserted_; }  // No phase check
  void Set(bool asserted) { asserted_ = asserted; }  // No phase check
  // ...
};
```

The design specifies that controls should validate their phase during access:
- `WriteControl.value()` should only be readable during Write phase
- `ReadControl.value()` should only be readable during Read phase
- `Control.Set()` should only be callable during Control phase

This validation enables microcode optimizations mentioned in design.md (lines 425-430).

**Impact**: Silent bugs possible when controls accessed in wrong phase; optimizer opportunities lost.

**Fix Plan**: See Issue 3 in [Fix Plan](#fix-plan) section.

---

### Issue 4: String-Based Path Resolution Contradicts Typed Navigation

**Location**: [microcode/include/irata2/microcode/ir/cpu_path_resolver.h](microcode/include/irata2/microcode/ir/cpu_path_resolver.h)
**Severity**: MEDIUM
**Status**: RESOLVED
**Design Violation**: "No string paths. All navigation is strongly typed" (design.md:277)

**Problem**:
```cpp
// Design goal (line 280):
auto& mar = cpu().memory().mar();  // Strongly typed

// Current implementation:
const ControlBase* ResolveControl(std::string_view path) const;  // String paths
std::vector<std::string> AllControlPaths() const;
```

While string-based resolution is needed for microcode compilation (YAML→IR), its presence in the HDL public API blurs the typed navigation design.

**Impact**: API confusion; potential misuse of string paths in production code.

**Resolution**: String path resolution now lives in `microcode::ir::CpuPathResolver`,
and HDL Cpu exposes only strongly typed accessors.

---

### Issue 5: Simulator Has Similar Mutable Cache Pattern

**Location**: [sim/include/irata2/sim/cpu.h:110,123-125](sim/include/irata2/sim/cpu.h#L123-L125)
**Severity**: MEDIUM

**Problem**:
```cpp
std::vector<Component*> components_;

mutable bool controls_indexed_ = false;
mutable std::unordered_map<std::string, ControlBase*> controls_by_path_;
mutable std::vector<std::string> control_paths_;
```

Same lazy initialization pattern as HDL, creating similar concerns about hidden state changes.

**Impact**: Inconsistent initialization timing; potential for bugs during concurrent access.

**Fix Plan**: See Issue 5 in [Fix Plan](#fix-plan) section.

---

## Minor Issues

### Issue 6: Bus Read-After-Write Not Explicitly Tracked

**Location**: [sim/include/irata2/sim/bus.h](sim/include/irata2/sim/bus.h)
**Severity**: LOW

The bus correctly validates phases and throws if reading before writing, but doesn't explicitly track that both operations happened in the same tick. Current implementation is correct but could be clearer.

### Issue 7: Controller Control Targets Pattern

**Location**: [sim/include/irata2/sim/controller.h](sim/include/irata2/sim/controller.h)
**Severity**: LOW

The controller uses a `control_targets_` map for control assertion, which works but could be more hardware-ish. Consider using the ROM-based approach described in design.md.

### Issue 8: Incomplete Status Register Wiring

**Location**: Various
**Severity**: LOW

Status register exists in HDL with 6502-style flags, but full bus connectivity for push/pop operations is documented as "planned" in READMEs. This is expected for MVP but should be tracked.

---

## Strengths

### Excellent Module Separation

The codebase demonstrates clear separation of concerns:

```
base (no deps) → hdl → sim
                  ↓
                 isa → microcode → assembler
```

Each module has a clear responsibility:
- **base**: Core types only (Byte, Word, TickPhase)
- **hdl**: Structural metadata, no runtime state
- **isa**: YAML-driven instruction definitions
- **sim**: Runtime execution with mutable state
- **microcode**: Compilation pipeline with transforms and validators
- **assembler**: Independent pipeline with clear stages

### Strong Type Safety

Byte and Word types are properly used throughout with no raw integer leakage:

```cpp
// base/include/irata2/base/types.h
class Byte {
  constexpr explicit Byte(uint8_t value) : value_(value) {}
  // Strong typing prevents accidental misuse
};
```

### Well-Structured Microcode Compiler

The compiler follows a clean multi-pass architecture:
1. FetchTransformer - Adds fetch preamble
2. SequenceTransformer - Adds SC management
3. FetchValidator - Validates uniform preamble
4. SequenceValidator - Validates SC discipline
5. IsaCoverageValidator - Verifies all instructions defined

### Good Test Infrastructure

Test files demonstrate good practices:
- Test helpers hide complexity
- Parameterized tests for variations
- E2E tests for assembly programs
- Test count: 43+ test files

---

## Fix Plan (Revised 2026-01-10)

Based on clarified design requirements, here is the refined implementation plan:

### Phase 1: HDL Singleton and Traits-Based Controls

#### Fix 1: Add Thread-Safe HDL Singleton
**Effort**: 1-2 hours
**Status**: DONE
**Files**:
- `hdl/include/irata2/hdl/cpu.h`
- `hdl/include/irata2/hdl/hdl.h` (update with GetCpu())
- `hdl/src/cpu.cpp`
- `hdl/test/singleton_test.cpp` (new)

**Approach**:
```cpp
// hdl/include/irata2/hdl/hdl.h
namespace irata2::hdl {
  // Thread-safe lazy initialization (C++11 guarantees)
  const Cpu& GetCpu();
}

// hdl/src/cpu.cpp
const Cpu& GetCpu() {
  static const Cpu instance;
  return instance;
}
```

#### Fix 2: Replace ControlBase Virtuals with ControlInfo Struct
**Effort**: 3-4 hours
**Status**: DONE
**Files**:
- `hdl/include/irata2/hdl/control_info.h` (new)
- `hdl/include/irata2/hdl/control_base.h` (delete)
- `hdl/include/irata2/hdl/control.h`
- `microcode/include/irata2/microcode/ir/step.h`
- All files using `const ControlBase*`

**Approach**:
```cpp
// hdl/include/irata2/hdl/control_info.h
struct ControlInfo {
  base::TickPhase phase;
  bool auto_reset;
  std::string_view path;  // Points to stable ComponentBase::path_
  // No virtuals - just POD-like data
};

// hdl/include/irata2/hdl/control.h
template <typename Derived, typename ValueType, base::TickPhase Phase, bool AutoReset>
class Control : public ComponentWithParent<Derived>, public ControlTag {
public:
  Control(std::string name, ComponentBase& parent)
      : ComponentWithParent<Derived>(std::move(name), parent),
        control_info_{Phase, AutoReset, ComponentWithParent<Derived>::path()} {}

  const ControlInfo& control_info() const { return control_info_; }

  static constexpr base::TickPhase kPhase = Phase;
  static constexpr bool kAutoReset = AutoReset;

private:
  const ControlInfo control_info_;
};
```

#### Fix 3: Remove Mutable Caches and Path Resolution from HDL Cpu
**Effort**: 1-2 hours
**Status**: DONE
**Files**:
- `hdl/include/irata2/hdl/cpu.h`
- `hdl/src/cpu.cpp`

**Approach**: Remove all mutable members and path resolution methods. HDL Cpu becomes truly immutable with strongly-typed accessors only.

### Phase 2: Microcode Path Resolver

#### Fix 4: Create CpuPathResolver in Microcode Module
**Effort**: 2-3 hours
**Status**: DONE
**Files**:
- `microcode/include/irata2/microcode/ir/cpu_path_resolver.h` (new)
- `microcode/src/ir/cpu_path_resolver.cpp` (new)
- `microcode/include/irata2/microcode/ir/builder.h` (update)
- `microcode/src/ir/builder.cpp` (update)
- `microcode/test/cpu_path_resolver_test.cpp` (new)

**Approach**:
```cpp
namespace irata2::microcode::ir {

class CpuPathResolver {
public:
  explicit CpuPathResolver(const hdl::Cpu& cpu);

  const hdl::ControlInfo* FindControl(std::string_view path) const;
  const hdl::ControlInfo* RequireControl(std::string_view path,
                                         std::string_view context) const;
  std::vector<std::string> AllControlPaths() const;

private:
  std::unordered_map<std::string, const hdl::ControlInfo*> controls_by_path_;
  std::vector<std::string> control_paths_;
};

}
```

### Phase 3: Parallel Visitor Encoding

#### Fix 5: Update ControlEncoder to Use Parallel Traversal
**Effort**: 3-4 hours
**Files**:
- `microcode/include/irata2/microcode/encoder/control_encoder.h`
- `microcode/src/encoder/control_encoder.cpp`
- `microcode/test/encoder/control_encoder_test.cpp`

**Approach**: Encoder traverses HDL and assigns bit indices by visit order. Sim control mapping uses parallel traversal with the same visit order.

### Phase 4: Simulator Updates

#### Fix 6: Add Phase Validation to Simulator Controls
**Effort**: 2-3 hours
**Files**:
- `sim/include/irata2/sim/control.h`
- `sim/test/control_test.cpp`

**Approach**: Validate phase on `asserted()` reads and `Set()` calls.

#### Fix 7: Remove Mutable Caches from Simulator
**Effort**: 1-2 hours
**Files**:
- `sim/include/irata2/sim/cpu.h`
- `sim/src/cpu.cpp`

### Phase 5: Testing

#### Fix 8: Add Comprehensive Tests
**Effort**: 2-3 hours
**Files**:
- `hdl/test/singleton_test.cpp` (new)
- `hdl/test/control_info_test.cpp` (new)
- `microcode/test/ir/cpu_path_resolver_test.cpp` (new)
- `sim/test/phase_validation_test.cpp` (new)

---

## Implementation Timeline

| Phase | Tasks | Effort | Priority |
|-------|-------|--------|----------|
| 1 | Fixes 1-3 (Singleton, ControlInfo, Immutability) | 5-8 hours | Must Have (Completed) |
| 2 | Fix 4 (CpuPathResolver) | 2-3 hours | Must Have (Completed) |
| 3 | Fix 5 (Parallel Traversal) | 3-4 hours | Must Have |
| 4 | Fixes 6-7 (Sim Updates) | 3-5 hours | Should Have |
| 5 | Fix 8 (Testing) | 2-3 hours | Must Have |
| **Total** | | **15-23 hours** | |

### Implementation Order

1. **Fix 2** (ControlInfo) - Core type change, many dependencies
2. **Fix 3** (Remove mutable) - Depends on Fix 2
3. **Fix 1** (Singleton) - Can be done with 2-3
4. **Fix 4** (CpuPathResolver) - Depends on Fix 2
5. **Fix 5** (Parallel Traversal) - Depends on Fix 4
6. **Fixes 6-7** (Sim) - Independent of HDL changes
7. **Fix 8** (Testing) - Throughout

Completed to date: Fixes 1-4.

---

## Conclusion

The IRATA2 codebase is **well-architected** with strong type safety, clean module boundaries, and good test infrastructure. The identified issues are **architectural refinements** rather than fundamental flaws, and can be addressed incrementally without major rewrites.

The most critical fixes (ControlBase CRTP refactor and mutable cache elimination) are now complete, restoring the foundational guarantees of zero-cost abstractions and immutability that the design relies on.

Remaining alignment work focuses on simulator phase validation and sim-side path caching. After those updates, the codebase will fully align with its stated design philosophy: a hardware-ish, type-safe, testable CPU simulator with compile-time verified correctness.

---

## Appendix: Files Reviewed

### Core Implementation Files
- `base/include/irata2/base/types.h` - Byte/Word types
- `base/include/irata2/base/tick_phase.h` - TickPhase enum
- `hdl/include/irata2/hdl/cpu.h` - HDL CPU structure
- `hdl/include/irata2/hdl/control_info.h` - Control metadata struct
- `hdl/include/irata2/hdl/control.h` - Control hierarchy
- `hdl/include/irata2/hdl/component.h` - CRTP base classes
- `sim/include/irata2/sim/cpu.h` - Simulator CPU
- `sim/include/irata2/sim/control.h` - Simulator controls
- `sim/include/irata2/sim/bus.h` - Simulator buses
- `microcode/include/irata2/microcode/ir/step.h` - Microcode IR
- `microcode/include/irata2/microcode/compiler/compiler.h` - Compiler

### Documentation Files
- `README.md` - Project overview
- `docs/design.md` - Architecture document
- `docs/plan.md` - Vertical slice roadmap
- `CLAUDE.md` - Claude Code instructions
- Module-specific READMEs

### Test Files
- 43+ test files across all modules
- E2E tests: `hlt.asm`, `nop.asm`, `crs.asm`
