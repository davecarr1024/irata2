# Code Review Checklist

Items to verify during code review or when auditing existing code.

## Hardware-ish Violations

### Hidden State in Controls

**Pattern to find:** Controls that store state internally rather than in explicit child registers.

**Example:** `ProgramCounter.add_offset` was a `ReadControl` that read from the bus and stored the value internally. Fixed in Phase 8 by adding explicit `signed_offset` register.

**How to detect:**
- Look for `ReadControl` on components that also have `ProcessControl` operations using that data
- Check if sim implementations store values read from buses in member variables
- Any control that "remembers" something between ticks should have a corresponding register

**Files to audit:**
- [x] `hdl/include/irata2/hdl/program_counter.h` - Known violation, fix in Phase 8

## Checklist for New Components

- [ ] All mutable state is in explicit registers visible in HDL
- [ ] No hidden state in control implementations
- [ ] Controls only use data from child registers or buses
- [ ] `visit_impl()` visits all child components
- [ ] Sim implementation mirrors HDL structure
