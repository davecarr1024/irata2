; Test STX instruction (Store X zero page)
LDX #$37
STX $10
LDA #$00
LDA $10
CMP #$37
JEQ stx_basic_ok
CRS
stx_basic_ok:
LDX #$00
STX $11
LDA #$FF
LDA $11
JEQ stx_zero_ok
CRS
stx_zero_ok:
HLT
