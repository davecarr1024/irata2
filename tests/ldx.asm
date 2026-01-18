; Test LDX instruction (Load X Register Immediate)
; Basic load
LDX #$42
TXA
CMP #$42
JEQ ldx_basic_ok
CRS
ldx_basic_ok:
; Load zero
LDX #$00
TXA
CMP #$00
JEQ ldx_zero_ok
CRS
ldx_zero_ok:
; Load max value
LDX #$FF
TXA
CMP #$FF
JEQ ldx_max_ok
CRS
ldx_max_ok:
; Independent registers
LDA #$11
LDX #$22
CMP #$11
JEQ ldx_indep_ok
CRS
ldx_indep_ok:
; Zero page load
LDA #$77
STA $10
LDX #$00
LDX $10
TXA
CMP #$77
JEQ ldx_zp_ok
CRS
ldx_zp_ok:
; Zero page load zero
LDA #$00
STA $11
LDX #$FF
LDX $11
TXA
CMP #$00
JEQ ldx_zp_zero_ok
CRS
ldx_zp_zero_ok:
; Absolute load
LDA #$33
STA $0200
LDX #$00
LDX $0200
TXA
CMP #$33
JEQ ldx_abs_ok
CRS
ldx_abs_ok:
HLT
