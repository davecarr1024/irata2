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

```mermaid
sequenceDiagram
    participant Controller
    participant Components
    participant Buses

    rect rgb(200, 220, 255)
        Note over Controller,Buses: Control Phase
        Controller->>Components: Assert control signals
    end

    rect rgb(255, 220, 200)
        Note over Controller,Buses: Write Phase
        Components->>Buses: Write values
    end

    rect rgb(220, 255, 200)
        Note over Controller,Buses: Read Phase
        Buses->>Components: Read values
    end

    rect rgb(255, 255, 200)
        Note over Controller,Buses: Process Phase
        Components->>Components: Internal updates (ALU, flags)
    end

    rect rgb(240, 240, 240)
        Note over Controller,Buses: Clear Phase
        Components->>Components: Reset auto-clear controls
    end
```

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
- Controller with IR/SC and control assertion from microcode programs
- Memory model with ROM/RAM mapping and MAR
- Status views for SR bits plus a status analyzer register for Z/N
- `irata2_run` CLI for executing cartridges
- End-to-end `.asm` test coverage (HLT/NOP/CRS) via `ctest`

Future additions:
- Expanded instruction coverage beyond HLT/NOP/CRS
- Additional addressing modes and branching
- Status register (SR) wiring for push/pop (planned)

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
#include "irata2/hdl.h"
#include "irata2/sim.h"

hdl::Cpu hdl;
sim::Cpu cpu(hdl);

// Execute cycles
while (!cpu.halted()) {
  cpu.Tick();
}

std::cout << "Executed " << cpu.cycle_count() << " cycles\n";
```

## Debugging CLI

The `irata2_run` helper can load debug sidecars and emit a failure dump:

```bash
irata2_run --debug program.json --trace-depth 64 program.bin
```

On unexpected crash/halt or timeout, the simulator prints a register/bus dump
plus a trace of recent instructions. Use `--expect-crash` to mark a crash as
expected or `--max-cycles N` to force a timeout.

## Files

- `component.h` - Base classes for sim components
- `cpu.h` / `cpu.cpp` - Root simulator with tick orchestration
