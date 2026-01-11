# ASM Test Programs

Each instruction in the ISA has a dedicated `.asm` program in this directory.
Every program exercises multiple cases for that instruction using simple
pass/fail control flow:

- Expected success paths continue to the next case.
- Expected failure paths trigger `CRS`.
- Each file ends in `HLT` (except `crs.asm`, which intentionally crashes).

Limitations:
- Some flags cannot be directly asserted yet (only `JEQ` is available),
  so tests focus on observable behavior with the current ISA.
