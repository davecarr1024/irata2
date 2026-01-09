# Sim Module - Runtime Simulator

The simulator module adds runtime execution capabilities to the HDL structure.

## Overview

While the HDL module describes the **structure** of the CPU, the sim module adds:
- Mutable runtime state (register values, bus contents)
- Five-phase tick orchestration
- Phase-aware control signal enforcement

The sim module takes an HDL Cpu as input and creates a running simulator that can execute instructions cycle-by-cycle.

## Five-Phase Tick Model

Each CPU clock cycle consists of five phases:

1. **Control**: Controller reads microcode and asserts control signals
2. **Write**: Components write their values to buses
3. **Read**: Components read values from buses
4. **Process**: Internal updates (ALU operations, status flag updates)
5. **Clear**: Reset auto-clear controls for next cycle

```cpp
sim::Cpu cpu(hdl_cpu);
cpu.Tick();  // Executes all five phases
```

## Key Concepts

### Phase Enforcement

Controls are only valid during their designated phase:
- Write controls: Only active during Write phase
- Read controls: Only active during Read phase
- Process controls: Only active during Process phase

Attempting to access a control in the wrong phase raises an error.

### Single Writer Enforcement

Each bus can only have one writer per tick. The simulator validates this at runtime.

### Auto-Reset vs Latched Controls

- **Auto-reset controls** clear after each tick (most control signals)
- **Latched controls** maintain their value until explicitly changed

## Current Status

The sim module currently provides:
- Basic Cpu class with tick orchestration
- Phase tracking
- Cycle counting
- Halt/resume functionality
- Core component types (controls, buses, registers, counters)
- Controller shell with IR and step counter

Future additions:
- Microcode ROM decoding and control assertion
- Memory access and fetch/decode wiring
- Halt/crash termination semantics for integration tests
- End-to-end `.asm` test runner integration (see `plan.md`)
 - Status register (SR) with 6502-style flags, bus-connected for push/pop

## Design Note: Controller Instruction Memory

The simulator controller will emulate a hardware-ish instruction memory:

- The microcode compiler outputs a **sparse** table keyed by `(opcode, step, status)` that maps to control sets.
- The controller **burns** this into a fully populated ROM image that maps the full address space of the controller (opcode/step/status) to a control word.
- The controller reads `controller.ir` (opcode), `controller.sc` (step), and status flags, encodes them into an instruction-memory address, and asserts the decoded control lines each tick.

This is the “spark” of the system: microcode becomes a physical ROM and drives the controller’s behavior directly. Encoders for status and control words will live alongside the controller as subcomponents.

## Memory Mapping Notes (MVP)

- Cartridge ROM image is `0x0000-0x7FFF`.
- The sim memory module maps cartridge ROM to CPU address space `0x8000-0xFFFF` at initialization.
- Unmapped memory reads return `0xFF` (CRS opcode) and should hard-fail tests that run off the end.

### Reference Example: pirata Controller

The `pirata` project uses a controller design worth emulating:

- **Controller owns IR + sequence counter + instruction memory**.
- **InstructionMemory.initialize(encoder)** burns the sparse microcode table into ROM chips sized for 16-bit addressing; if the table exceeds 16-bit addressing, it falls back to a sparse in-memory table.
- **Multi-ROM control words**: control words are split across multiple ROM chips, one byte per ROM, and reassembled on read.
- **Tick behavior**: `tick_control` reads `(opcode, step, statuses)`, fetches the control set from instruction memory, and asserts each control line; `tick_process` latches instruction boundaries when the sequence counter resets.

This is the exact “burn in” pattern we want: a sparse table from the compiler is turned into a ROM-backed instruction memory that drives the controller each tick.

## Usage

```cmake
target_link_libraries(your_target PRIVATE irata2::sim)
```

```cpp
#include "irata2/hdl/cpu.h"
#include "irata2/sim/cpu.h"

hdl::Cpu hdl;
sim::Cpu cpu(hdl);

// Execute cycles
while (!cpu.halted()) {
  cpu.Tick();
}

std::cout << "Executed " << cpu.cycle_count() << " cycles\n";
```

## Files

- `component.h` - Base classes for sim components
- `cpu.h` / `cpu.cpp` - Root simulator with tick orchestration
