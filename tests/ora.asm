; Test ORA instruction
LDA #$F0
ORA #$0F
CMP #$FF
JEQ ora_ok
CRS
ora_ok:
; Test ORA with zero identity
LDA #$42
ORA #$00
CMP #$42
JEQ ora_zero_ok
CRS
ora_zero_ok:
; Zero page ORA
LDA #$0F
STA $10
LDA #$F0
ORA $10
CMP #$FF
JEQ ora_zp_ok
CRS
ora_zp_ok:
HLT
