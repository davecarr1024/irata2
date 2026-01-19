; Test LDX instruction (Load X Register)
; Immediate load: reset A before TXA to avoid no-op pass
LDX #$42
LDA #$00
TXA
CMP #$41
JEQ ldx_basic_fail
CMP #$42
JEQ ldx_basic_ok
CRS
ldx_basic_ok:
; Immediate load zero: A must become 0x00 after TXA
LDX #$00
LDA #$55
TXA
CMP #$01
JEQ ldx_zero_fail
CMP #$00
JEQ ldx_zero_ok
CRS
ldx_zero_ok:
; Immediate load max: ensure 0xFF arrives
LDX #$FF
LDA #$00
TXA
CMP #$FE
JEQ ldx_max_fail
CMP #$FF
JEQ ldx_max_ok
CRS
ldx_max_ok:
; Independent registers: loading X does not clobber A
LDA #$11
LDX #$22
CMP #$11
JEQ ldx_indep_ok
CRS
ldx_indep_ok:
; Zero page load: read from memory, then reset A before TXA
LDA #$77
STA $10
LDX #$00
LDX $10
LDA #$00
TXA
CMP #$76
JEQ ldx_zp_fail
CMP #$77
JEQ ldx_zp_ok
CRS
ldx_zp_ok:
; Zero page load zero: ensure X becomes 0x00
LDA #$00
STA $11
LDX #$FF
LDX $11
LDA #$55
TXA
CMP #$01
JEQ ldx_zp_zero_fail
CMP #$00
JEQ ldx_zp_zero_ok
CRS
ldx_zp_zero_ok:
; Absolute load: read from 0x0200
LDA #$33
STA $0200
LDX #$00
LDX $0200
LDA #$00
TXA
CMP #$32
JEQ ldx_abs_fail
CMP #$33
JEQ ldx_abs_ok
CRS
ldx_abs_ok:
; Zero page Y-indexed load
LDY #$02
LDA #$44
STA $12
LDX $10, Y
LDA #$00
TXA
CMP #$43
JEQ ldx_zpy_fail
CMP #$44
JEQ ldx_zpy_ok
CRS
ldx_zpy_ok:
; Absolute Y-indexed load
LDY #$01
LDA #$55
STA $0301
LDX $0300, Y
LDA #$00
TXA
CMP #$54
JEQ ldx_aby_fail
CMP #$55
JEQ ldx_aby_ok
CRS
ldx_aby_ok:
HLT
ldx_basic_fail:
CRS
ldx_zero_fail:
CRS
ldx_max_fail:
CRS
ldx_zp_fail:
CRS
ldx_zp_zero_fail:
CRS
ldx_abs_fail:
CRS
ldx_zpy_fail:
CRS
ldx_aby_fail:
CRS
