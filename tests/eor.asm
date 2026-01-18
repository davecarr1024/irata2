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
; Zero page EOR
LDA #$F0
STA $10
LDA #$FF
EOR $10
CMP #$0F
JEQ eor_zp_ok
CRS
eor_zp_ok:
; Absolute EOR
LDA #$0F
STA $0200
LDA #$F0
EOR $0200
CMP #$FF
JEQ eor_abs_ok
CRS
eor_abs_ok:
HLT
