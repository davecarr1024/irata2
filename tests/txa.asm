; Test TXA instruction (Transfer X to A)
; Basic transfer: reset A so TXA must overwrite it
LDX #$42
LDA #$00
TXA
CMP #$41
JEQ txa_basic_fail
CMP #$42
JEQ txa_basic_ok
CRS
txa_basic_ok:
; Transfer zero: A must be cleared by TXA
LDX #$00
LDA #$55
TXA
CMP #$01
JEQ txa_zero_fail
CMP #$00
JEQ txa_zero_ok
CRS
txa_zero_ok:
; Transfer max value: ensure 0xFF arrives
LDX #$FF
LDA #$00
TXA
CMP #$FE
JEQ txa_max_fail
CMP #$FF
JEQ txa_max_ok
CRS
txa_max_ok:
HLT
txa_basic_fail:
CRS
txa_zero_fail:
CRS
txa_max_fail:
CRS
