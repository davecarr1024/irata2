# Assembler Design (MVP Pipeline)

This assembler is structured as a strict pipeline so each stage has a single
responsibility and produces stable artifacts for debugging and tests.

## Goals

- Minimal MVP to assemble `HLT`, `NOP`, and `CRS` for end-to-end testing.
- Predictable address layout with implied `.org $8000` entry point.
- Deterministic output for binary cartridge ROM plus debug metadata.
- Clear separation between syntax, binding, and encoding.

## Pipeline Stages

### 1) Lexer

Input: raw source text. Output: token stream with source spans.

Token types:
- Identifiers (labels, mnemonics).
- Numeric literals (hex like `$8000`, binary like `%1010`, decimal).
- Punctuation (`:`, `,`, newline).
- Directives (prefixed with `.` like `.org`).
- Comments (discarded but tracked for spans if needed).

Output also tracks line/column and original text slice for error messages.

### 2) Parser

Input: tokens. Output: AST.

AST nodes:
- `Program`: ordered list of statements.
- `LabelDecl`: `name:`.
- `Instruction`: mnemonic + raw operands (unbound).
- `Directive`: `.org`, `.byte` (MVP).
- `Blank/Comment`: optional if needed for debug spans.

### 3) First Pass (Bind + Layout)

Input: AST. Output: partially bound IR + symbol table.

Responsibilities:
- Establish memory layout (address per statement).
- Record label locations.
- Bind instruction shapes and operand kinds but leave label references unresolved.
- Validate instruction count against ISA (HLT/NOP/CRS only for MVP).
- Produce a linear list of `Emittable` records with address + source span.

Artifacts:
- `SymbolTable`: label → address.
- `Layout`: ordered list of `Emittable` with placeholder operands.

### 4) Second Pass (Resolve)

Input: layout + symbol table. Output: fully bound IR.

Responsibilities:
- Resolve label references to concrete addresses.
- Validate address ranges and alignment requirements.
- Fail fast on unknown labels or out-of-range targets.

### 5) Encode

Input: fully bound IR. Output: binary + debug table.

Responsibilities:
- Emit raw bytes with a fixed cartridge header.
- Apply memory mapping offset (program address space starts at 0x8000,
  ROM is 0x0000-based).
- Create debug entries for each instruction:
  - source span (file, line, column).
  - address in CPU space and ROM offset.
  - mnemonic + operands.

## Data Structures (Sketch)

- `Token { kind, span }`
- `Span { file, line, column, length }`
- `AstNode` variants above.
- `InstructionIR { address, opcode, operands, span }`
- `DebugEntry { address, rom_offset, span, text }`
- `AssemblerResult { rom_bytes, debug_entries }`

## Error Handling

- All errors include a source span and a short, specific message.
- First-pass errors stop before second-pass/encode.
- Second-pass errors are reserved for label resolution and range checks.

## MVP Assumptions

- Default implied `.org $8000` if not specified.
- Only `HLT`, `NOP`, `CRS` mnemonics are accepted.
- No branching yet, but label resolution is built-in for future growth.
- Cartridge ROM size defaults to 0x8000 unless otherwise specified.
- Symbols and mnemonics are case-insensitive (normalized to lowercase).
- Numeric literals support `$` hex, `%` binary, and decimal.

## Cartridge Header (MVP)

The cartridge file begins with a fixed 32-byte header:

- Magic: `IRTA` (4 bytes).
- Version: u16 (little-endian), currently `1`.
- Header size: u16 (little-endian), always `32`.
- Entry point: u16 (little-endian), default `0x8000`.
- ROM size: u32 (little-endian), default `0x8000`.
- Reserved: 18 bytes, all zero.

## Integration Points

- ISA definition from `isa/instructions.yaml` drives opcode mapping.
- Cartridge format is fixed-header + binary payload, debug symbols optional.
- Sim loads the ROM bytes into the cartridge region (0x8000 mapped).
 
## Debug Output (MVP)

Debug data is emitted as JSON with:

- header info (version, entry, rom_size)
- record list (address, rom_offset, line, column, text)
