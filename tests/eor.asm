; Test EOR instruction
LDA #$FF
EOR #$0F
CMP #$F0
JEQ eor_ok
CRS
eor_ok:
; Test EOR with same value gives zero
LDA #$42
EOR #$42
JEQ eor_zero_ok
CRS
eor_zero_ok:
HLT
