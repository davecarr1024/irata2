; Test TXA instruction (Transfer X to A)
; Basic transfer
LDX #$42
TXA
CMP #$42
JEQ txa_basic_ok
CRS
txa_basic_ok:
; Transfer zero
LDX #$00
TXA
CMP #$00
JEQ txa_zero_ok
CRS
txa_zero_ok:
; Transfer max value
LDX #$FF
TXA
CMP #$FF
JEQ txa_max_ok
CRS
txa_max_ok:
HLT
