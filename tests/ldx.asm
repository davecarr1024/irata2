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
HLT
