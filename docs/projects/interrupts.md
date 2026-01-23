# IRQ Design Proposal (Based on Pirata)

## Goals

- Inject an IRQ entry opcode at instruction start only.
- Keep IRQ logic contained in Controller + Instruction Register.
- Provide a CPU-level IRQ control line for MMIO devices to drive.
- Preserve existing microcode flow; add a dedicated IRQ entry opcode.

## Summary of the Pirata Model

Pirata implements IRQ injection by:
- Latching an `instruction_complete` signal when the sequence counter resets.
- Using a specialized `InstructionRegister` with an output mux:
  - If `instruction_complete & irq_line & ~I_flag`, it returns a synthetic
    IRQ opcode (0xFF in Pirata).
  - Otherwise it returns the opcode fetched from memory.
- Wiring `cpu.irq_line` (latched control) into device factories so MMIO devices
  can assert interrupts without CPU-side polling logic.

This keeps IRQ behavior out of the CPU tick loop and avoids ad-hoc checks
outside the controller/IR path.

## Proposed Architecture for IRATA2

### 1) IRQ Line Plumbing (CPU + Memory factories)

- Add `LatchedProcessControl irq_line` to `sim::Cpu`.
- Extend memory region factories to accept `irq_line` so MMIO devices can drive it.
  - Proposed signature:
    `using RegionFactory = std::function<std::unique_ptr<Region>(Memory&, LatchedProcessControl&)>;`
  - CPU builds RAM/ROM factories with the extra arg, and forwards the same
    `irq_line` to all extra region factories.

### 2) Instruction Boundary Detection (Controller)

- Add a `LatchedProcessControl instruction_complete` to `Controller`.
- In `Controller::TickProcess`, set it when `sc.reset` is asserted.
- Clear the latch after it has been consumed (either in `TickProcess` or a
  dedicated clear step mirroring Pirata).

This tracks instruction boundaries with existing microcode controls, no CPU
state variables.

### 3) Interrupt-Aware Instruction Register

Introduce `InstructionRegister` (subclass of `ByteRegister`) with:
- Input pins:
  - `irq_line` (from CPU)
  - `instruction_complete` (from Controller)
  - `status.interrupt_disable` (I flag)
- Internal latch `inject_interrupt` (latched process control).
- Output mux: if `inject_interrupt` is true, `value()` returns synthetic
  IRQ opcode; otherwise it returns stored bus opcode.

**Timing:**
- Process phase: if `instruction_complete`, latch `inject_interrupt`
  using `irq_line & ~I`.
- Read phase: load real opcode from bus.
- Next Control phase: controller reads `ir.value()` and sees muxed opcode.

This satisfies “only at instruction start” and keeps logic in controller/IR.

### 4) IRQ Entry Opcode + Microcode

- Add a synthetic opcode `IRQ_ENTRY` to the ISA table.
  - Decision: use `0x00` for IRQ entry.
- Microcode for `IRQ_ENTRY` mirrors BRK entry but:
  - B flag is cleared in the pushed status.
  - I flag is set (interrupt_disable).
  - Vector at `$FFFE/FFFF`.

### 5) Device Integration

MMIO devices accept the CPU `irq_line` and drive it as:
- `irq_line.assert` when input becomes available (queue transitions empty→non-empty).
- `irq_line.clear` when device is serviced (queue becomes empty).

This is compatible with the existing input device behavior and allows
interrupt-driven input for demo programs.

## Device IRQ Policy (Default)

For queue-based MMIO devices, default to level-sensitive IRQ:
- Assert when queue transitions from empty to non-empty.
- Keep asserted while queue has data.
- Clear when queue becomes empty.

Non-queue devices should document their own IRQ policy explicitly.

## Control Type Hierarchy

- `sim` already has `LatchedControl` and `ProcessControl<false>` variants.
- Requirement: add explicit `LatchedControl` types in `hdl` (mirroring sim),
  plus unit tests verifying latch/clear semantics in both layers.

## Instruction Start Signal

Decision: replace `controller.ipc.latch` with a dedicated
`controller.instruction_start` control that fires on the first microcode step.

This control can:
- Drive IPC latching.
- Drive IR mux decision (IRQ injection).
- Provide a single, named instruction-boundary signal for future features.

## Proposed Tests

1) **InstructionRegister unit tests**
   - When instruction_complete=1 and irq_line=1 and I=0:
     - IR returns IRQ opcode during Control phase.
   - When I=1 or irq_line=0:
     - IR returns fetched opcode.
2) **Controller instruction boundary test**
   - Assert `sc.reset` and verify instruction_complete latch toggles.
3) **CPU region factory test**
   - Device factory receives IRQ line and can drive it.
4) **End-to-end IRQ test (assembly)**
   - Program loops; device asserts IRQ; handler runs and clears device.
5) **BRK vs IRQ distinction**
   - Handler checks B flag pushed by BRK vs IRQ entry.

## Full IRQ Integration Test Plan (MMIO → IRQ → Handler → RTI)

**Goal:** Verify that a simulated MMIO device asserts IRQ, CPU vectors to the
handler on instruction start, executes handler, and returns via RTI to the main
program.

**Test setup (C++ integration test):**
1) Create a fake MMIO device module (e.g. `TestIrqDevice`) with:
   - A status register at `$5000` (bit 0 = pending).
   - A data register at `$5001` (read clears pending).
   - Access to `irq_line` control (latched control from CPU).
2) Device behavior:
   - On construction, pending=false.
   - Method `Trigger()` sets pending=true and asserts `irq_line`.
   - On read of `$5001`, clear pending and deassert `irq_line`.

**Program under test (assembly):**
- Install IRQ vector at `$FFFE` to `irq_handler`.
- Main loop increments a counter at `$0000` continuously.
- IRQ handler:
  - Reads `$5001` (clears pending).
  - Increments a counter at `$0001` (tracks IRQ hits).
  - RTI.

**Test flow:**
1) Assemble program to ROM; construct CPU with:
   - RAM/ROM regions
   - TestIrqDevice region factory
2) Run CPU for N cycles to confirm main loop is active.
3) Call `device.Trigger()` and continue running.
4) Assert:
   - `$0001` incremented at least once (IRQ handler ran).
   - `$0000` continues incrementing after handler returns (RTI successful).
   - `irq_line` cleared after handler reads `$5001`.

**Assertions/coverage:**
- Interrupt only taken at instruction boundary (handler does not interrupt mid-step).
- IRQ masked when I flag is set (optional second test: set I and re-trigger).
- IRQ line is level-sensitive (kept high until device read clears it).

## Rollout Plan

1) Add `InstructionRegister` and wire it in `Controller` (no behavior change yet).
2) Add `instruction_complete` latch and IRQ line plumbing.
3) Add IRQ opcode + microcode for IRQ entry.
4) Add tests (unit + integration).
5) Update input device to assert IRQ line via factory-provided control.

## Open Questions

1) **Device IRQ policy defaults**: For each MMIO device, should we default to
   asserting on empty→non-empty and clearing on empty, unless overridden by the
   device spec?
