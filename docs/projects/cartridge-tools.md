# Cartridge Tools and Inspection

## Goals

- Make cartridges easy to inspect, validate, and disassemble.
- Provide tooling to diagnose cartridge/test failures quickly.

## Current State

- Cartridge header parsing exists in `cartridge.cpp`.

## Proposed Work

1. **Cartridge inspection CLI**
   - Dump header/metadata (size, entry point, checksum if any).
   - Validate layout and required sections.

2. **Disassembler**
   - Decode cartridge bytes into assembly.
   - Integrate symbols if a debug sidecar is present.

3. **Program metadata**
   - Optionally embed or reference build info in the cartridge.

4. **Test integration**
   - Add tests that confirm round-trip assemble -> disassemble.

## Milestones

- M0: Basic cartridge dump and sanity checks.
- M1: Disassembler with labels and offsets.
- M2: Optional symbol-aware disassembly.

## Open Questions

- Where should this CLI live (assembler vs a new tool)?
