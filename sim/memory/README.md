# Sim Memory Design

The memory subsystem mirrors the pirata design, adapted to the C++ sim
component model. Memory is a CPU component connected directly to the data bus
and owns a Memory Address Register (MAR) that connects to both the address bus
and the data bus (low/high bytes).

## Component Layout

- `cpu.memory`
  - Read/write controls on the data bus.
  - `memory.mar`
    - Word read/write on the address bus.
    - `memory.mar.low` and `memory.mar.high` for byte transfers on the data bus.

## Regions and Modules

Memory is divided into regions. Each region:
- Covers a contiguous address range.
- Has a power-of-2 size and aligned base address.
- Wraps a memory module and translates addresses into the module’s local space.
- Is validated as non-overlapping when memory is constructed.

Modules implement a simple interface:
- `size()`
- `Read(address)`
- `Write(address, value)`

RAM and ROM modules are provided; ROM throws on write.

## Default Layout

- RAM region at `0x0000-0x1FFF` (8 KB).
- Cartridge ROM mapped at `0x8000` (32 KB).
- Unmapped reads return `0xFF` (CRS opcode); unmapped writes are errors.

## CPU Integration

The CPU constructor accepts:
- A cartridge ROM module.
- Optional extra regions for MMIO devices.

These are passed into `memory` on construction so the simulator stays hardware-ish.

## Pirata Reference

This mirrors `pirata/sim/components/memory/memory.py`:
- Regions with non-overlap validation.
- Module-based address translation.
- MAR-driven bus I/O.
