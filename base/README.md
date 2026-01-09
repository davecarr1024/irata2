# Base Module

Fundamental types and utilities used across all IRATA2 modules.

## Overview

The base module provides strongly-typed value types for 8-bit and 16-bit values, and the tick phase model that governs CPU execution.

## Types

### Byte

An 8-bit value type (0x00 - 0xFF) with:
- Constexpr construction and operations
- Arithmetic operations (+, -, &, |, ^, ~, <<, >>)
- Comparison operations
- String formatting via `to_string()`

```cpp
#include "irata2/base/types.h"
using namespace irata2::base;

Byte a(0x10);
Byte b(0x20);
Byte c = a + b;  // c.value() == 0x30
```

### Word

A 16-bit value type (0x0000 - 0xFFFF) with:
- Constexpr construction and operations
- Construction from high/low bytes: `Word(Byte high, Byte low)`
- Byte extraction: `high()` and `low()`
- Same arithmetic/comparison operations as Byte

```cpp
Word w(Byte(0x12), Byte(0x34));  // w.value() == 0x1234
Byte hi = w.high();              // hi.value() == 0x12
Byte lo = w.low();               // lo.value() == 0x34
```

### TickPhase

An enum representing the five phases of each CPU clock cycle:

```cpp
enum class TickPhase {
  None,     // Not in a tick
  Control,  // Controller reads microcode, asserts control signals
  Write,    // Components write to buses
  Read,     // Components read from buses
  Process,  // Internal updates (ALU, status flags)
  Clear     // Reset auto-clear controls
};
```

The `ToString(TickPhase)` function converts phases to readable strings for debugging.

## Usage

```cmake
target_link_libraries(your_target PRIVATE irata2::base)
```

```cpp
#include "irata2/base/types.h"
#include "irata2/base/tick_phase.h"
```

## Design Notes

- **Strong typing**: `Byte` and `Word` are not aliases to raw integers - they're distinct types that prevent accidental misuse
- **Constexpr**: All operations are compile-time evaluable
- **Overflow/underflow**: Values wrap naturally (like hardware)
- **Header-only**: No linking required beyond include path
