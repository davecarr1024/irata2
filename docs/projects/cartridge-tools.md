# Cartridge Tools and Inspection

## Goals

- Make cartridges easy to inspect, validate, and disassemble.
- Provide tooling to diagnose cartridge/test failures quickly.

## Current State

- Cartridge header parsing exists in `cartridge.cpp`.
- `irata2_cart` dumps cartridge headers and optional debug metadata.
- `irata2_disasm` disassembles ROMs with optional debug symbols.

## Proposed Work

1. **Cartridge inspection CLI**
   - `irata2_cart --rom game.bin [--debug game.json]`
   - Dumps header metadata (magic, version, entry, rom size) and debug summary.

2. **Disassembler**
   - `irata2_disasm --rom game.bin [--debug game.json]`
   - Optional flags: `--show-addresses`, `--show-bytes`, `--no-labels`, `--no-org`.

3. **Program metadata**
   - Debug sidecar supplies symbols/source metadata today (cartridge embed is still optional).

4. **Test integration**
   - Round-trip assemble/disassemble test lives in `sim/test/disassembler_test.cpp`.

## Milestones

- M0: Basic cartridge dump and sanity checks.
- M1: Disassembler with labels and offsets.
- M2: Optional symbol-aware disassembly.

## Open Questions

- Where should this CLI live (assembler vs a new tool)?
