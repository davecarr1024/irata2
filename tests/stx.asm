; Test STX instruction (Store X)
; Zero page store
LDX #$37
STX $10
LDA #$00
LDA $10
CMP #$36
JEQ stx_basic_fail
CMP #$37
JEQ stx_basic_ok
CRS
stx_basic_ok:
; Zero page store zero
LDX #$00
STX $11
LDA #$FF
LDA $11
CMP #$01
JEQ stx_zero_fail
CMP #$00
JEQ stx_zero_ok
CRS
stx_zero_ok:
; Absolute store
LDX #$55
STX $0200
LDA #$00
LDA $0200
CMP #$54
JEQ stx_abs_fail
CMP #$55
JEQ stx_abs_ok
CRS
stx_abs_ok:
; Zero page Y-indexed store
LDY #$02
LDX #$22
STX $10, Y
LDA #$00
LDA $12
CMP #$21
JEQ stx_zpy_fail
CMP #$22
JEQ stx_zpy_ok
CRS
stx_zpy_ok:
; Absolute Y-indexed store
LDY #$01
LDX #$33
STX $0300, Y
LDA #$00
LDA $0301
CMP #$32
JEQ stx_aby_fail
CMP #$33
JEQ stx_aby_ok
CRS
stx_aby_ok:
HLT
stx_basic_fail:
CRS
stx_zero_fail:
CRS
stx_abs_fail:
CRS
stx_zpy_fail:
CRS
stx_aby_fail:
CRS
