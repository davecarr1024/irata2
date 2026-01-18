; Test STA instruction (Store A zero page)
LDA #$42
STA $10
LDA #$00
LDA $10
CMP #$42
JEQ sta_basic_ok
CRS
sta_basic_ok:
LDA #$00
STA $11
LDA #$FF
LDA $11
JEQ sta_zero_ok
CRS
sta_zero_ok:
HLT
