# IRATA2 Assembler

A 6502-style assembler for the IRATA2 CPU simulator. This assembler converts assembly language source files into cartridge ROM images that can be run in the simulator.

## Quick Start

```bash
# Assemble a program
./build/assembler/irata2_asm myprogram.asm output.bin output.json

# Run in the simulator
./build/frontend/irata2_demo --rom output.bin
```

## Assembly Language Syntax

### Instructions

Instructions use standard 6502 mnemonics. Case-insensitive.

```asm
LDA #$42        ; Load immediate value into A
STA $0200       ; Store A at address $0200
JMP loop        ; Jump to label
```

### Labels

Labels mark locations in code for jumps and branches.

```asm
start:
    LDA #$00

loop:
    INX
    CPX #$10
    BNE loop        ; Branch back to loop
    JMP start       ; Jump to start
```

### Comments

Comments begin with `;` and continue to end of line.

```asm
LDA #$FF    ; This is a comment
; Full line comment
```

## Directives

### `.org` - Set Origin Address

Sets the address where following code will be assembled. Default is `$8000`.

```asm
.org $8000      ; Code starts at $8000 (default entry point)
```

### `.byte` - Define Raw Bytes

Embeds raw byte values in the output. Supports multiple comma-separated values.

```asm
.byte $00, $FF, $42     ; Three bytes
.byte %10101010         ; Binary literal
.byte 255               ; Decimal literal

; Data tables
dir_table:
    .byte $00, $01, $02, $01, $00, $FF, $FE, $FF
```

### `.equ` - Define Constants

Creates named constants that can be used anywhere a numeric value is expected.

```asm
; Hardware addresses
.equ VGC_CMD, $4100
.equ VGC_COLOR, $4105
.equ VGC_EXEC, $4106

; Color constants
.equ COLOR_BLACK, $00
.equ COLOR_WHITE, $03

; Use constants in code
LDA #COLOR_WHITE        ; Immediate value
STA VGC_COLOR           ; Address
```

### `.include` - Include Files

Includes another assembly file at the current location. Circular includes are detected and prevented.

```asm
.include "hardware.asm"     ; Include hardware definitions
.include "utils/math.asm"   ; Relative paths supported
```

## Numeric Literals

| Format | Example | Value |
|--------|---------|-------|
| Hex | `$FF`, `$8000` | 255, 32768 |
| Binary | `%1010`, `%11110000` | 10, 240 |
| Decimal | `42`, `255` | 42, 255 |

## Addressing Modes

### Implied (IMP)

No operand. The instruction operates on a register or has implicit behavior.

```asm
NOP             ; No operation
RTS             ; Return from subroutine
CLC             ; Clear carry
INX             ; Increment X
```

### Immediate (IMM) `#value`

Operand is a literal value.

```asm
LDA #$42        ; A = $42
LDX #$FF        ; X = $FF
ADC #$01        ; A = A + 1 + carry
AND #$0F        ; A = A & $0F
```

### Zero Page (ZP) `$xx`

Addresses in the first 256 bytes ($00-$FF). Faster than absolute addressing.

```asm
LDA $10         ; A = memory[$10]
STA $00         ; memory[$00] = A
INC $20         ; memory[$20]++
```

### Zero Page Indexed (ZPX, ZPY) `$xx,X` or `$xx,Y`

Zero page address plus index register.

```asm
LDA $10,X       ; A = memory[$10 + X]
STA $20,Y       ; memory[$20 + Y] = A
INC $30,X       ; memory[$30 + X]++
```

### Absolute (ABS) `$xxxx`

Full 16-bit address.

```asm
LDA $0200       ; A = memory[$0200]
STA $4100       ; memory[$4100] = A
JMP $8000       ; PC = $8000
```

### Absolute Indexed (ABX, ABY) `$xxxx,X` or `$xxxx,Y`

16-bit address plus index register.

```asm
LDA $0200,X     ; A = memory[$0200 + X]
STA $0300,Y     ; memory[$0300 + Y] = A
```

### Indirect (IND) `($xxxx)`

Jump to address stored at the given location.

```asm
JMP ($FFFC)     ; PC = memory[$FFFC] (16-bit)
```

### Indexed Indirect (IZX) `($xx,X)`

Pointer in zero page, indexed by X before dereferencing.

```asm
LDA ($20,X)     ; ptr = memory[$20+X], A = memory[ptr]
```

### Indirect Indexed (IZY) `($xx),Y`

Pointer in zero page, indexed by Y after dereferencing.

```asm
LDA ($30),Y     ; ptr = memory[$30], A = memory[ptr+Y]
STA ($40),Y     ; ptr = memory[$40], memory[ptr+Y] = A
```

### Relative (REL) `label` or `offset`

Signed 8-bit offset for branch instructions (-128 to +127 bytes).

```asm
loop:
    DEX
    BNE loop    ; Branch if not zero (backward)

    BEQ skip    ; Branch if zero (forward)
    NOP
skip:
```

## Instruction Reference

### Load/Store

| Instruction | Description | Modes |
|-------------|-------------|-------|
| `LDA` | Load A | IMM, ZP, ZPX, ABS, ABX, ABY, IZX, IZY |
| `LDX` | Load X | IMM, ZP, ZPY, ABS, ABY |
| `LDY` | Load Y | IMM, ZP, ZPX, ABS, ABX |
| `STA` | Store A | ZP, ZPX, ABS, ABX, ABY, IZX, IZY |
| `STX` | Store X | ZP, ZPY, ABS |
| `STY` | Store Y | ZP, ZPX, ABS |

### Arithmetic

| Instruction | Description | Modes |
|-------------|-------------|-------|
| `ADC` | Add with carry | IMM, ZP, ZPX, ABS, ABX, ABY, IZX, IZY |
| `SBC` | Subtract with carry | IMM, ZP, ZPX, ABS, ABX, ABY, IZX, IZY |
| `INC` | Increment memory | ZP, ZPX, ABS, ABX |
| `DEC` | Decrement memory | ZP, ZPX, ABS, ABX |
| `INX` | Increment X | IMP |
| `DEX` | Decrement X | IMP |
| `INY` | Increment Y | IMP |
| `DEY` | Decrement Y | IMP |

### Logic

| Instruction | Description | Modes |
|-------------|-------------|-------|
| `AND` | Logical AND | IMM, ZP, ZPX, ABS, ABX, ABY, IZX, IZY |
| `ORA` | Logical OR | IMM, ZP, ZPX, ABS, ABX, ABY, IZX, IZY |
| `EOR` | Exclusive OR | IMM, ZP, ZPX, ABS, ABX, ABY, IZX, IZY |
| `BIT` | Bit test | ZP, ABS |

### Shift/Rotate

| Instruction | Description | Modes |
|-------------|-------------|-------|
| `ASL` | Arithmetic shift left | IMP, ZP, ZPX, ABS, ABX |
| `LSR` | Logical shift right | IMP, ZP, ZPX, ABS, ABX |
| `ROL` | Rotate left through carry | IMP, ZP, ZPX, ABS, ABX |
| `ROR` | Rotate right through carry | IMP, ZP, ZPX, ABS, ABX |

### Compare

| Instruction | Description | Modes |
|-------------|-------------|-------|
| `CMP` | Compare A | IMM, ZP, ZPX, ABS, ABX, ABY, IZX, IZY |
| `CPX` | Compare X | IMM, ZP, ABS |
| `CPY` | Compare Y | IMM, ZP, ABS |

### Transfer

| Instruction | Description |
|-------------|-------------|
| `TAX` | A to X |
| `TXA` | X to A |
| `TAY` | A to Y |
| `TYA` | Y to A |
| `TSX` | Stack pointer to X |
| `TXS` | X to stack pointer |

### Branch

| Instruction | Condition |
|-------------|-----------|
| `BEQ` | Equal (Z=1) |
| `BNE` | Not equal (Z=0) |
| `BCS` | Carry set (C=1) |
| `BCC` | Carry clear (C=0) |
| `BMI` | Minus (N=1) |
| `BPL` | Plus (N=0) |
| `BVS` | Overflow set (V=1) |
| `BVC` | Overflow clear (V=0) |

### Jump/Call

| Instruction | Description | Modes |
|-------------|-------------|-------|
| `JMP` | Jump | ABS, IND |
| `JSR` | Jump to subroutine | ABS |
| `RTS` | Return from subroutine | IMP |

### Stack

| Instruction | Description |
|-------------|-------------|
| `PHA` | Push A |
| `PLA` | Pull A |
| `PHP` | Push processor status |
| `PLP` | Pull processor status |

### Status Flags

| Instruction | Description |
|-------------|-------------|
| `CLC` | Clear carry |
| `SEC` | Set carry |
| `CLV` | Clear overflow |

### System

| Instruction | Description |
|-------------|-------------|
| `NOP` | No operation |
| `HLT` | Halt CPU |
| `BRK` | Software interrupt |

## Subroutines

Use `JSR` to call subroutines and `RTS` to return. Initialize the stack pointer first.

```asm
.org $8000

; Initialize stack
LDX #$FF
TXS

; Main code
LDA #$10
JSR double      ; Call subroutine
STA $00         ; A is now $20
HLT

; Subroutine: doubles A
double:
    ASL         ; A = A << 1
    RTS
```

### Preserving Registers

Use the stack to save and restore registers in subroutines.

```asm
my_routine:
    PHA         ; Save A
    ; ... do work ...
    PLA         ; Restore A
    RTS
```

## Hardware Interface

### Input Port ($4000-$4002)

| Address | Description |
|---------|-------------|
| `$4000` | Input available flag (bit 0) |
| `$4002` | Key code |

Key codes: 1=up, 2=down, 3=left, 4=right, $20=space

```asm
check_input:
    LDA $4000
    AND #$01
    BEQ no_input    ; No input available

    LDA $4002       ; Read key code
    CMP #$01
    BEQ handle_up
    ; ...

no_input:
    RTS
```

### Vector Graphics Coprocessor ($4100-$4107)

| Address | Description |
|---------|-------------|
| `$4100` | Command (1=clear, 2=point, 3=line) |
| `$4101` | X0 coordinate |
| `$4102` | Y0 coordinate |
| `$4103` | X1 coordinate (line) |
| `$4104` | Y1 coordinate (line) |
| `$4105` | Color (0-3 intensity) |
| `$4106` | Execute command |
| `$4107` | Frame complete |

```asm
; Clear screen to black
LDA #$01        ; CLEAR command
STA $4100
LDA #$00        ; Black
STA $4105
LDA #$01        ; Execute
STA $4106

; Draw white point at (100, 50)
LDA #$02        ; POINT command
STA $4100
LDA #$64        ; X = 100
STA $4101
LDA #$32        ; Y = 50
STA $4102
LDA #$03        ; White
STA $4105
LDA #$01        ; Execute
STA $4106

; Draw line from (10, 20) to (50, 60)
LDA #$03        ; LINE command
STA $4100
LDA #$0A        ; X0 = 10
STA $4101
LDA #$14        ; Y0 = 20
STA $4102
LDA #$32        ; X1 = 50
STA $4103
LDA #$3C        ; Y1 = 60
STA $4104
LDA #$03        ; White
STA $4105
LDA #$01        ; Execute
STA $4106

; Signal frame complete
LDA #$02
STA $4107
```

## Memory Map

| Range | Size | Description |
|-------|------|-------------|
| `$0000-$00FF` | 256B | Zero page (fast access) |
| `$0100-$01FF` | 256B | Stack |
| `$0200-$3FFF` | ~16KB | RAM |
| `$4000-$41FF` | 512B | I/O ports |
| `$8000-$FFFF` | 32KB | ROM (cartridge) |

## Example: Complete Program

```asm
; Hardware constants
.equ VGC_CMD, $4100
.equ VGC_X0, $4101
.equ VGC_Y0, $4102
.equ VGC_COLOR, $4105
.equ VGC_EXEC, $4106
.equ VGC_FRAME, $4107

.equ CMD_CLEAR, $01
.equ CMD_POINT, $02
.equ COLOR_BLACK, $00
.equ COLOR_WHITE, $03

; Zero page variables
.equ pos_x, $00
.equ pos_y, $01

.org $8000

; Initialize
    LDX #$FF
    TXS             ; Set up stack

    LDA #$80        ; Start at center
    STA pos_x
    STA pos_y

; Main loop
loop:
    JSR clear_screen
    JSR draw_dot
    JSR frame_done
    JMP loop

; Subroutine: Clear screen
clear_screen:
    LDA #CMD_CLEAR
    STA VGC_CMD
    LDA #COLOR_BLACK
    STA VGC_COLOR
    LDA #$01
    STA VGC_EXEC
    RTS

; Subroutine: Draw dot at pos_x, pos_y
draw_dot:
    LDA #CMD_POINT
    STA VGC_CMD
    LDA pos_x
    STA VGC_X0
    LDA pos_y
    STA VGC_Y0
    LDA #COLOR_WHITE
    STA VGC_COLOR
    LDA #$01
    STA VGC_EXEC
    RTS

; Subroutine: Signal frame complete
frame_done:
    LDA #$02
    STA VGC_FRAME
    RTS
```

## Cartridge Format

The assembled binary starts with a 32-byte header:

| Offset | Size | Description |
|--------|------|-------------|
| 0 | 4 | Magic: `IRTA` |
| 4 | 2 | Version (1) |
| 6 | 2 | Header size (32) |
| 8 | 2 | Entry point |
| 10 | 4 | ROM size |
| 14 | 18 | Reserved |

## Debug Output

The assembler produces a JSON file with debug information:

```json
{
  "header": {
    "version": 1,
    "entry": 32768,
    "rom_size": 32768
  },
  "symbols": {
    "loop": 32771,
    "clear_screen": 32780
  },
  "debug": [
    {"address": 32768, "line": 20, "text": "LDX #$FF"}
  ]
}
```

## Error Messages

The assembler provides detailed error messages with file, line, and column information:

```
error: unknown mnemonic 'LDQ'
  --> myfile.asm:15:5
   |
15 |     LDQ #$42
   |     ^^^
```

Common errors:
- Unknown mnemonic
- Invalid addressing mode for instruction
- Undefined label
- Duplicate label
- Branch target out of range (-128 to +127)
- Invalid numeric literal
